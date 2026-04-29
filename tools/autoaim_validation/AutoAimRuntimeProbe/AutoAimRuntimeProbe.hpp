#pragma once

// clang-format off
/* === MODULE MANIFEST V2 ===
module_description: Runtime counter probe for camera/sync/detector/tracker validation
constructor_args: []
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
  - qdu-future/ArmorDetector
  - qdu-future/ArmorTracker
=== END MANIFEST === */
// clang-format on

#include <atomic>
#include <cstdint>

#include "ArmorDetector.hpp"
#include "ArmorTracker.hpp"
#include "CameraBase.hpp"
#include "SolveTrajectory.hpp"
#include "app_framework.hpp"
#include "libxr.hpp"
#include "logger.hpp"
#include "message.hpp"

template <CameraTypes::CameraInfo CameraInfoV>
class AutoAimRuntimeProbe : public LibXR::Application
{
 public:
  using Base = CameraBase<CameraInfoV>;
  using ImuStamped = typename Base::ImuStamped;
  using GyroStamped = typename Base::GyroStamped;
  using AcclStamped = typename Base::AcclStamped;
  using QuatStamped = typename Base::QuatStamped;
  using DetectionFrame = ArmorDetectionsFrameMessage<CameraInfoV>;
  using DetectorMetrics = ArmorDetectorMetrics;
  using Tracker = ArmorTracker<CameraInfoV>;
  using CandidateDebug = typename Tracker::CandidateDebugMsg;
  using Target = SolveTrajectory::Target;

  AutoAimRuntimeProbe(LibXR::HardwareContainer&, LibXR::ApplicationManager& app)
  {
    RegisterDefaultTopic("camera_gyro", raw_gyro_count_);
    RegisterDefaultTopic("camera_accl", raw_accl_count_);
    RegisterDefaultTopic("camera_quat", raw_quat_count_);
    RegisterDefaultTopic("camera_imu", synced_imu_count_);
    RegisterDomainTopic("armors_frame", armor_detector_domain_, detector_frame_count_);
    RegisterDomainTopic("metrics", armor_detector_domain_, detector_metrics_count_);
    RegisterDomainTopic("target", tracker_domain_, tracker_target_count_);
    RegisterDomainTopic("candidate_debug", tracker_domain_, tracker_candidate_debug_count_);
    app.Register(*this);
  }

  void OnMonitor() override
  {
    XR_LOG_PASS(
        "PIPELINE_PROBE raw_gyro=%llu raw_accl=%llu raw_quat=%llu synced_imu=%llu "
        "det_frame=%llu det_metrics=%llu tracker_target=%llu tracker_candidate=%llu",
        static_cast<unsigned long long>(raw_gyro_count_.load()),
        static_cast<unsigned long long>(raw_accl_count_.load()),
        static_cast<unsigned long long>(raw_quat_count_.load()),
        static_cast<unsigned long long>(synced_imu_count_.load()),
        static_cast<unsigned long long>(detector_frame_count_.load()),
        static_cast<unsigned long long>(detector_metrics_count_.load()),
        static_cast<unsigned long long>(tracker_target_count_.load()),
        static_cast<unsigned long long>(tracker_candidate_debug_count_.load()));
  }

 private:
  void RegisterDefaultTopic(const char* name, std::atomic<uint64_t>& counter)
  {
    auto handle = LibXR::Topic::WaitTopic(name, 1000);
    if (handle == nullptr)
    {
      XR_LOG_WARN("AutoAimRuntimeProbe topic missing: %s", name);
      return;
    }
    LibXR::Topic topic(handle);
    auto cb = LibXR::Topic::Callback::Create(CountCallback, &counter);
    topic.RegisterCallback(cb);
  }

  void RegisterDomainTopic(const char* name, LibXR::Topic::Domain& domain,
                           std::atomic<uint64_t>& counter)
  {
    auto handle = LibXR::Topic::WaitTopic(name, 1000, &domain);
    if (handle == nullptr)
    {
      XR_LOG_WARN("AutoAimRuntimeProbe domain topic missing: %s", name);
      return;
    }
    LibXR::Topic topic(handle);
    auto cb = LibXR::Topic::Callback::Create(CountCallback, &counter);
    topic.RegisterCallback(cb);
  }

  static void CountCallback(bool, std::atomic<uint64_t>* counter, LibXR::RawData&)
  {
    counter->fetch_add(1, std::memory_order_relaxed);
  }

 private:
  LibXR::Topic::Domain armor_detector_domain_{"armor_detector"};
  LibXR::Topic::Domain tracker_domain_{"tracker"};
  std::atomic<uint64_t> raw_gyro_count_{0};
  std::atomic<uint64_t> raw_accl_count_{0};
  std::atomic<uint64_t> raw_quat_count_{0};
  std::atomic<uint64_t> synced_imu_count_{0};
  std::atomic<uint64_t> detector_frame_count_{0};
  std::atomic<uint64_t> detector_metrics_count_{0};
  std::atomic<uint64_t> tracker_target_count_{0};
  std::atomic<uint64_t> tracker_candidate_debug_count_{0};
};
