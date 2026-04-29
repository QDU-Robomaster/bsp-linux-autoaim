#pragma once

// clang-format off
/* === MODULE MANIFEST V2 ===
module_description: File-backed camera source for internal capture files with embedded quaternion headers
constructor_args:
  - runtime:
      file_path: "/home/xiao/data/camera_internal_recording_20260428/damo.avi"
      camera_name: "camera"
      image_topic_name: "camera_image"
      imu_topic_name: "camera_imu"
      realtime: true
      loop: false
      clear_embedded_header: true
template_args:
  - Info:
      width: 1440
      height: 1080
      step: 4320
      encoding: CameraTypes::Encoding::BGR8
      camera_matrix: [2328.6857198980888, 0.0, 733.35646250924742, 0.0, 2328.6701077899961, 540.61872869227727, 0.0, 0.0, 1.0]
      distortion_model: CameraTypes::DistortionModel::PLUMB_BOB
      distortion_coefficients: [-0.091821039187099038, 0.46399073468302049, 0.0026098786426372819, 0.0009819586010405485, -0.47512788503104569]
      rectification_matrix: [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]
      projection_matrix: [2328.6857198980888, 0.0, 733.35646250924742, 0.0, 0.0, 2328.6701077899961, 540.61872869227727, 0.0, 0.0, 0.0, 1.0, 0.0]
required_hardware: []
depends:
  - qdu-future/CameraBase
=== END MANIFEST === */
// clang-format on

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "CameraBase.hpp"
#include "app_framework.hpp"
#include "libxr.hpp"
#include "logger.hpp"
#include "message.hpp"
#include "thread.hpp"

template <CameraTypes::CameraInfo CameraInfoV>
class CaptureFileCamera : public LibXR::Application,
                          public CameraBase<CameraInfoV>
{
 public:
  using Self = CaptureFileCamera<CameraInfoV>;
  using Base = CameraBase<CameraInfoV>;
  using ImageFrame = typename Base::ImageFrame;
  using GyroStamped = typename Base::GyroStamped;
  using AcclStamped = typename Base::AcclStamped;
  using QuatStamped = typename Base::QuatStamped;

  static inline constexpr auto camera_info = Base::camera_info;
  static constexpr int channel_count = 3;
  static constexpr std::size_t header_bytes = sizeof(double) * 5U;
  static constexpr std::size_t frame_step = static_cast<std::size_t>(camera_info.step);
  static constexpr int frame_width = static_cast<int>(camera_info.width);
  static constexpr int frame_height = static_cast<int>(camera_info.height);

  static_assert(camera_info.encoding == CameraTypes::Encoding::BGR8,
                "CaptureFileCamera currently publishes BGR8 frames");
  static_assert(frame_step == static_cast<std::size_t>(camera_info.width) * channel_count,
                "CaptureFileCamera expects tightly packed BGR8 frames");

  struct RuntimeParam
  {
    std::string_view file_path =
        "/home/xiao/data/camera_internal_recording_20260428/damo.avi";
    std::string_view camera_name = "camera";
    std::string_view image_topic_name = "camera_image";
    std::string_view imu_topic_name = "camera_imu";
    bool realtime = true;
    bool loop = false;
    bool clear_embedded_header = true;
    uint32_t max_frames = 0;
  };

  struct EmbeddedHeader
  {
    double time{};
    double w{};
    double x{};
    double y{};
    double z{};
  };

  explicit CaptureFileCamera(LibXR::HardwareContainer& hw,
                             LibXR::ApplicationManager& app,
                             RuntimeParam runtime)
      : Base(hw, runtime.camera_name, runtime.image_topic_name, runtime.imu_topic_name),
        file_path_(runtime.file_path),
        runtime_(runtime),
        gyro_topic_name_(std::string(this->Name()) + "_gyro"),
        accl_topic_name_(std::string(this->Name()) + "_accl"),
        quat_topic_name_(std::string(this->Name()) + "_quat"),
        raw_gyro_topic_(LibXR::Topic::FindOrCreate<GyroStamped>(gyro_topic_name_.c_str())),
        raw_accl_topic_(LibXR::Topic::FindOrCreate<AcclStamped>(accl_topic_name_.c_str())),
        raw_quat_topic_(LibXR::Topic::FindOrCreate<QuatStamped>(quat_topic_name_.c_str()))
  {
    ApplyEnvironmentOverrides();
    ValidateVideo();
    running_.store(true);
    capture_thread_.Create(this, CaptureThreadMain, "CaptureFileCam",
                           static_cast<size_t>(1024 * 256),
                           LibXR::Thread::Priority::HIGH);
    app.Register(*this);
  }

  ~CaptureFileCamera() override { running_.store(false); }

  void OnMonitor() override
  {
    XR_LOG_INFO("CaptureFileCamera monitor: frames=%u imu=%u running=%d",
                frames_committed_.load(), imu_published_.load(), running_.load() ? 1 : 0);
  }

  void SetExposure(double) override {}
  void SetGain(double) override {}

 private:
  static bool ReadHeader(const cv::Mat& frame, EmbeddedHeader& header)
  {
    if (frame.empty() || frame.data == nullptr ||
        frame.total() * frame.elemSize() < header_bytes)
    {
      return false;
    }
    std::memcpy(&header, frame.data, sizeof(header));
    return std::isfinite(header.time) && std::isfinite(header.w) &&
           std::isfinite(header.x) && std::isfinite(header.y) &&
           std::isfinite(header.z);
  }

  static std::array<double, 4> NormalizeQuaternion(const EmbeddedHeader& header)
  {
    std::array<double, 4> q{header.w, header.x, header.y, header.z};
    const double norm =
        std::sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    if (!std::isfinite(norm) || norm <= std::numeric_limits<double>::epsilon())
    {
      return {1.0, 0.0, 0.0, 0.0};
    }
    for (double& v : q)
    {
      v /= norm;
    }
    return q;
  }

  static std::array<float, 3> EstimateGyro(const std::array<double, 4>& prev,
                                           const std::array<double, 4>& cur,
                                           double dt_s)
  {
    if (!(dt_s > 0.0) || !std::isfinite(dt_s))
    {
      return {0.0F, 0.0F, 0.0F};
    }

    std::array<double, 4> q = cur;
    const double dot = prev[0] * q[0] + prev[1] * q[1] + prev[2] * q[2] + prev[3] * q[3];
    if (dot < 0.0)
    {
      for (double& v : q)
      {
        v = -v;
      }
    }

    const std::array<double, 4> inv_prev{prev[0], -prev[1], -prev[2], -prev[3]};
    const std::array<double, 4> dq{
        q[0] * inv_prev[0] - q[1] * inv_prev[1] - q[2] * inv_prev[2] - q[3] * inv_prev[3],
        q[0] * inv_prev[1] + q[1] * inv_prev[0] + q[2] * inv_prev[3] - q[3] * inv_prev[2],
        q[0] * inv_prev[2] - q[1] * inv_prev[3] + q[2] * inv_prev[0] + q[3] * inv_prev[1],
        q[0] * inv_prev[3] + q[1] * inv_prev[2] - q[2] * inv_prev[1] + q[3] * inv_prev[0],
    };

    const double v_norm = std::sqrt(dq[1] * dq[1] + dq[2] * dq[2] + dq[3] * dq[3]);
    if (v_norm <= 1e-12)
    {
      return {0.0F, 0.0F, 0.0F};
    }

    const double angle = 2.0 * std::atan2(v_norm, std::clamp(dq[0], -1.0, 1.0));
    const double scale = angle / (v_norm * dt_s);
    return {static_cast<float>(dq[1] * scale), static_cast<float>(dq[2] * scale),
            static_cast<float>(dq[3] * scale)};
  }

  static uint64_t DeltaUsFromHeader(const EmbeddedHeader& header, uint64_t fallback_us)
  {
    if (std::isfinite(header.time) && header.time > 0.0 && header.time < 1.0)
    {
      return static_cast<uint64_t>(std::llround(header.time * 1000000.0));
    }
    return fallback_us;
  }

  void ApplyEnvironmentOverrides()
  {
    if (const char* env = std::getenv("CAPTURE_FILE_CAMERA_MAX_FRAMES"))
    {
      const unsigned long parsed = std::strtoul(env, nullptr, 10);
      if (parsed > 0UL)
      {
        runtime_.max_frames = static_cast<uint32_t>(parsed);
      }
    }
    if (const char* env = std::getenv("CAPTURE_FILE_CAMERA_REALTIME"))
    {
      runtime_.realtime = !(env[0] == '0' && env[1] == '\0');
    }
  }

  void ValidateVideo()
  {
    cv::VideoCapture cap(file_path_);
    if (!cap.isOpened())
    {
      XR_LOG_ERROR("CaptureFileCamera failed to open '%s'", file_path_.c_str());
      throw std::runtime_error("CaptureFileCamera: failed to open file");
    }

    const int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    const int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    const double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps > 0.0 && std::isfinite(fps))
    {
      fallback_period_us_ = static_cast<uint64_t>(std::llround(1000000.0 / fps));
    }

    if (width != frame_width || height != frame_height)
    {
      XR_LOG_ERROR("CaptureFileCamera geometry mismatch: file=%dx%d constexpr=%dx%d",
                   width, height, frame_width, frame_height);
      throw std::runtime_error("CaptureFileCamera: geometry mismatch");
    }

    XR_LOG_PASS("CaptureFileCamera opened %s: %dx%d fps=%.3f period=%llu us",
                file_path_.c_str(), width, height, fps,
                static_cast<unsigned long long>(fallback_period_us_));
  }

  void WaitForImageSink()
  {
    while (running_.load() && !this->ImageSinkReady())
    {
      LibXR::Thread::Sleep(1);
    }
  }

  bool ConvertToBgr(const cv::Mat& decoded, cv::Mat& bgr)
  {
    if (decoded.empty())
    {
      return false;
    }
    if (decoded.type() == CV_8UC3)
    {
      bgr = decoded;
      return true;
    }
    if (decoded.type() == CV_8UC4)
    {
      cv::cvtColor(decoded, bgr, cv::COLOR_BGRA2BGR);
      return true;
    }
    if (decoded.type() == CV_8UC1)
    {
      cv::cvtColor(decoded, bgr, cv::COLOR_GRAY2BGR);
      return true;
    }

    XR_LOG_ERROR("CaptureFileCamera unsupported cv type=%d", decoded.type());
    return false;
  }

  void PublishRawImu(uint64_t timestamp_us, const std::array<double, 4>& quat,
                     const std::array<float, 3>& gyro)
  {
    GyroStamped gyro_msg{
        .sensor_timestamp_us = timestamp_us,
        .angular_velocity_xyz = {gyro[0], gyro[1], gyro[2]},
    };
    AcclStamped accl_msg{
        .sensor_timestamp_us = timestamp_us,
        .linear_acceleration_xyz = {0.0F, 0.0F, 0.0F},
    };
    QuatStamped quat_msg{
        .sensor_timestamp_us = timestamp_us,
        .rotation_wxyz = {static_cast<float>(quat[0]), static_cast<float>(quat[1]),
                          static_cast<float>(quat[2]), static_cast<float>(quat[3])},
    };

    raw_gyro_topic_.Publish(gyro_msg);
    raw_accl_topic_.Publish(accl_msg);
    raw_quat_topic_.Publish(quat_msg);
    imu_published_.fetch_add(1);
  }

  bool WriteAndCommitImage(const cv::Mat& bgr, uint64_t timestamp_us)
  {
    if (!this->ImageSinkReady())
    {
      return false;
    }

    ImageFrame* image = this->GetWritableImage();
    if (image == nullptr)
    {
      XR_LOG_WARN("CaptureFileCamera writable image is null");
      return false;
    }

    image->timestamp_us = timestamp_us;
    for (int row = 0; row < frame_height; ++row)
    {
      std::memcpy(image->data.data() + static_cast<std::size_t>(row) * frame_step,
                  bgr.ptr(row), frame_step);
    }

    if (runtime_.clear_embedded_header)
    {
      std::memset(image->data.data(), 0, header_bytes);
    }

    if (!this->CommitImage())
    {
      XR_LOG_WARN("CaptureFileCamera image commit failed");
      return false;
    }

    frames_committed_.fetch_add(1);
    return true;
  }

  bool ProcessFrame(const cv::Mat& decoded)
  {
    EmbeddedHeader header{};
    if (!ReadHeader(decoded, header))
    {
      XR_LOG_WARN("CaptureFileCamera skipped frame with invalid embedded header");
      return false;
    }

    cv::Mat bgr;
    if (!ConvertToBgr(decoded, bgr))
    {
      return false;
    }
    if (bgr.cols != frame_width || bgr.rows != frame_height || bgr.elemSize() != channel_count)
    {
      XR_LOG_ERROR("CaptureFileCamera decoded frame shape mismatch");
      return false;
    }

    const uint64_t delta_us = DeltaUsFromHeader(header, fallback_period_us_);
    timestamp_us_ += delta_us;
    const std::array<double, 4> quat = NormalizeQuaternion(header);
    const double dt_s = static_cast<double>(delta_us) / 1000000.0;
    const std::array<float, 3> gyro =
        previous_quat_valid_ ? EstimateGyro(previous_quat_, quat, dt_s)
                             : std::array<float, 3>{0.0F, 0.0F, 0.0F};
    previous_quat_ = quat;
    previous_quat_valid_ = true;

    PublishRawImu(timestamp_us_, quat, gyro);
    const bool committed = WriteAndCommitImage(bgr, timestamp_us_);

    if (runtime_.realtime)
    {
      const uint64_t sleep_ms = std::max<uint64_t>(1U, delta_us / 1000U);
      LibXR::Thread::Sleep(static_cast<uint32_t>(sleep_ms));
    }

    return committed;
  }

  void CaptureLoop()
  {
    WaitForImageSink();
    cv::VideoCapture cap(file_path_);
    if (!cap.isOpened())
    {
      XR_LOG_ERROR("CaptureFileCamera failed to open '%s' in worker", file_path_.c_str());
      running_.store(false);
      return;
    }

    cv::Mat decoded;
    while (running_.load())
    {
      if (!cap.read(decoded))
      {
        XR_LOG_PASS("CaptureFileCamera reached EOF after %u committed frames",
                    frames_committed_.load());
        if (!runtime_.loop)
        {
          running_.store(false);
          break;
        }
        cap.set(cv::CAP_PROP_POS_FRAMES, 0.0);
        timestamp_us_ = 0;
        previous_quat_valid_ = false;
        continue;
      }

      (void)ProcessFrame(decoded);
      if (runtime_.max_frames != 0U && frames_committed_.load() >= runtime_.max_frames)
      {
        XR_LOG_PASS("CaptureFileCamera reached max_frames=%u", runtime_.max_frames);
        running_.store(false);
        break;
      }
    }
  }

  static void CaptureThreadMain(Self* self) { self->CaptureLoop(); }

 private:
  std::string file_path_{};
  RuntimeParam runtime_{};
  std::string gyro_topic_name_{};
  std::string accl_topic_name_{};
  std::string quat_topic_name_{};
  LibXR::Topic raw_gyro_topic_{};
  LibXR::Topic raw_accl_topic_{};
  LibXR::Topic raw_quat_topic_{};
  LibXR::Thread capture_thread_{};
  std::atomic<bool> running_{false};
  std::atomic<uint32_t> frames_committed_{0};
  std::atomic<uint32_t> imu_published_{0};
  uint64_t fallback_period_us_{10000};
  uint64_t timestamp_us_{0};
  bool previous_quat_valid_{false};
  std::array<double, 4> previous_quat_{1.0, 0.0, 0.0, 0.0};
};
