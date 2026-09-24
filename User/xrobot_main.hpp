#pragma once
// xrobot-stamp: config=xrobot.yaml sha256=c686deefc6c88b76e4a54c2d33e88081c5156f34a04244216b2d905e40e4ded6
// xrobot-stamp: lock=../xrobot.lock sha256=ec47458d4271263753e102adb37c3cb326c07b32092bdbf279b1c9db2b589f7d

#include <memory>
#include <type_traits>
#include <utility>
#include "libxr.hpp"
#include "thread.hpp"
#include "HikCamera.hpp"
#include "CameraFrameSync.hpp"
#include "SharedTopic.hpp"
#include "ArmorDetector.hpp"
#include "ArmorTracker.hpp"
#include "Aimer.hpp"
#include "SharedTopicClient.hpp"
#include "CameraBase.hpp"

namespace xrobot_generated {
template <typename...> struct TypeList {};
template <typename Source, typename... Views>
struct RegistrationMatches
    : std::bool_constant<(!std::is_reference<Views>::value && ...) &&
                         (std::is_convertible<Source*, Views*>::value && ...)> {};

}  // namespace xrobot_generated

namespace AutoAimRunConfig {
inline constexpr float HikExposureTimeUs = 2000.0F;
inline constexpr int HikTriggerDelayUs = 75;
inline constexpr int HikSyncOffsetUs = HikTriggerDelayUs + static_cast<int>(HikExposureTimeUs * 0.5F);
inline constexpr CameraTypes::CameraCalibration MainCameraCalibration = {.native_width = 1440, .native_height = 1080, .camera_matrix = {2328.685719898089, 0.0, 733.3564625092474, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 1.0}, .distortion_model = CameraTypes::DistortionModel::PLUMB_BOB, .distortion_coefficients = {-0.09182103918709904, 0.4639907346830205, 0.002609878642637282, 0.0009819586010405485, -0.4751278850310457}, .rectification_matrix = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, .projection_matrix = {2328.685719898089, 0.0, 733.3564625092474, 0.0, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 0.0, 1.0, 0.0}};
inline constexpr CameraTypes::FrameLayout HikFrameLayout = {.width = 720, .height = 540, .step = 2160, .encoding = CameraTypes::Encoding::BGR8};
}  // namespace AutoAimRunConfig

// Force only this entry inline in optimized Clang builds.
#if defined(__clang__) && defined(__OPTIMIZE__) && !defined(LIBXR_DEBUG_BUILD) && \
    ((defined(XROBOT_OPTIMIZED_BUILD) && XROBOT_OPTIMIZED_BUILD) || \
     (!defined(XROBOT_OPTIMIZED_BUILD) && defined(NDEBUG)))
#define XR_XROBOT_MAIN_INLINE [[gnu::always_inline]] inline
#else
#define XR_XROBOT_MAIN_INLINE inline
#endif

[[noreturn]] XR_XROBOT_MAIN_INLINE void XRobotMain(
    LibXR::RamFS& ramfs,
    LibXR::UART& devc_usb) {
  // modules[0]: camera
  static HikCamera<AutoAimRunConfig::HikFrameLayout> camera(
      static_cast<LibXR::RamFS&>(ramfs)
      , AutoAimRunConfig::MainCameraCalibration
      , HikCamera<AutoAimRunConfig::HikFrameLayout>::RuntimeParam(
static_cast<std::string_view>("gimbal")
, static_cast<std::string_view>("camera_image")
, static_cast<std::string_view>("camera_imu")
, static_cast<float>(16.0F)
, static_cast<float>(AutoAimRunConfig::HikExposureTimeUs)
, static_cast<bool>(true)
, static_cast<float>(249.0F)
, static_cast<uint32_t>(100)
, static_cast<uint32_t>(3)
, static_cast<uint32_t>(2)
, static_cast<uint32_t>(2)
, static_cast<bool>(false)
, static_cast<std::optional<HikCamera<AutoAimRunConfig::HikFrameLayout>::AdcBitDepth>>(std::nullopt)
, static_cast<bool>(false)
, static_cast<float>(1.0F)
)
  );
  // modules[1]: camera_frame_sync
  static CameraFrameSync<AutoAimRunConfig::HikFrameLayout> camera_frame_sync(
      static_cast<CameraFrameSync<AutoAimRunConfig::HikFrameLayout>::Base&>(camera)
      , CameraFrameSync<AutoAimRunConfig::HikFrameLayout>::RuntimeParam(
static_cast<CameraFrameSync<AutoAimRunConfig::HikFrameLayout>::SyncMode>(CameraFrameSync<AutoAimRunConfig::HikFrameLayout>::SyncMode::RAW_PROBE)
, static_cast<int32_t>(AutoAimRunConfig::HikSyncOffsetUs)
, static_cast<std::string_view>("host")
, static_cast<std::string_view>("camera_sync_command")
, static_cast<std::string_view>("camera_sync_result")
, static_cast<uint32_t>(3)
, static_cast<uint32_t>(1)
, static_cast<float>(100.0F)
, static_cast<CameraFrameSync<AutoAimRunConfig::HikFrameLayout>::RawImuFrame>(CameraFrameSync<AutoAimRunConfig::HikFrameLayout>::RawImuFrame::BODY_X_RIGHT_Y_FORWARD_Z_UP)
, std::remove_cv_t<std::remove_reference_t<std::string_view>>{}
)
  );
  static std::initializer_list<SharedTopic::TopicConfig> xr_arg_shared_topic_rx_topic_configs =
      {
{
"gimbal_gyro"
, "host"
}
, {
"gimbal_accl"
, "host"
}
, {
"gimbal_quat"
, "host"
}
, {
"camera_sync_result"
, "host"
}
, {
"robot_game_ref"
, "host"
}
}
  ;
  // modules[2]: shared_topic_rx
  static SharedTopic shared_topic_rx(
      static_cast<LibXR::UART&>(devc_usb)
      , static_cast<LibXR::RamFS&>(ramfs)
      , "DevC-USB"
      , 4096
      , xr_arg_shared_topic_rx_topic_configs
  );
  // modules[3]: armor_detector
  static ArmorDetector<AutoAimRunConfig::HikFrameLayout> armor_detector(
      static_cast<ArmorDetector<AutoAimRunConfig::HikFrameLayout>::Sync&>(camera_frame_sync)
      , ArmorDetector<AutoAimRunConfig::HikFrameLayout>::Config{
.detect_color = 2
, .network = {
.model = ArmorDetectorModel::INT16_HEAD_L
, .min_confidence = 0.1
, .enable_quad_check = true
, .min_quad_area_px = 16.0
, .logit_threshold = 0.619
, .nms_threshold = 0.45
, .bbox_expand = 0.1
, .max_detections = 128
}
, .referee_auto_detect_color = true
, .referee_domain = "host"
, .referee_topic = "robot_game_ref"
, .preview = {
.enabled = false
, .preview_window_name = "armor_detector_preview"
, .preview_scale = 0.5
, .preview_wait_key_ms = 1
, .queue_capacity = 1
, .output_mode = "window"
, .web_bind_address = "0.0.0.0"
, .web_port = 8080
, .web_stream_name = "armor_detector"
, .max_fps = 30.0
}
, .number_refine = {}
}
  );
  // modules[4]: armor_tracker
  static ArmorTracker<AutoAimRunConfig::HikFrameLayout> armor_tracker(
      static_cast<LibXR::RamFS&>(ramfs)
      , static_cast<ArmorTracker<AutoAimRunConfig::HikFrameLayout>::FrameSync&>(camera_frame_sync)
      , ArmorTracker<AutoAimRunConfig::HikFrameLayout>::Config{
.tracker = {
.require_target_tag = false
, .target_tag_id = -1
, .min_detect_count = 2
, .max_temp_lost_count = 15
, .outpost_max_temp_lost_count = 75
, .target_select = {
.observed_count_weight = 1.6
, .distance_weight = 2.0
, .area_weight = 1.2
, .spin_weight = 0.8
, .angle_weight = 2.0
, .max_distance_m = 8.0
, .distance_span_m = 7.5
, .area_norm_px = 6000.0
, .observed_count_norm = 4.0
, .max_spin_rad_s = 8.0
, .max_angle_norm = 0.5
, .detecting_scale = 0.55
, .temp_lost_scale = 0.35
, .switch_margin = 0.25
}
}
, .extrinsic = {
.camera_mount_to_body = {
.rotation = {
0.999929746909
, -0.009747410407
, -0.004716638126
, -0.004821053970
}
, .translation = {
0.041861764663827829
, 0.136068364765315
, 0.0089956658836358675
}
}
}
, .preview = {
.enabled = false
, .preview_window_name = "armor_tracker_preview"
, .preview_scale = 0.5
, .preview_wait_key_ms = 1
, .queue_capacity = 1
, .output_mode = "window"
, .web_bind_address = "0.0.0.0"
, .web_port = 8080
, .web_stream_name = "armor_tracker"
, .max_fps = 30.0
}
}
  );
  // modules[5]: aimer
  static Aimer<AutoAimRunConfig::HikFrameLayout> aimer(
      Aimer<AutoAimRunConfig::HikFrameLayout>::Config{
.yaw_offset = 0.0
, .roll_offset = 0.6
, .yaw_rate_threshold = 2.0
, .default_bullet_speed = 21.7
, .min_valid_bullet_speed = 14.0
, .ballistic_drag_k = 0.02
, .ballistic_integration_dt_s = 0.001
, .ballistic_max_iterations = 16
, .ballistic_min_elevation_deg = -20.0
, .ballistic_max_elevation_deg = 35.0
, .auto_fire = true
, .image_to_now_s = 0.0
, .vision_to_command_delay_s = 0.0
, .command_transport_delay_s = 0.0
, .gimbal_response_delay_s = 0.0
, .fire_delay_s = 0.05
, .low_speed_extra_predict_s = 0.075
, .high_speed_extra_predict_s = 0.075
, .min_fire_threshold = 0.06
, .max_fire_threshold = 0.48
, .enable_mpc_plan = true
, .mpc_fire_thresh = 0.48
, .max_yaw_acc = 50.0
, .q_yaw_pos = 9000000.0
, .q_yaw_vel = 0.0
, .r_yaw_acc = 1.0
, .max_roll_acc = 100.0
, .q_roll_pos = 9000000.0
, .q_roll_vel = 0.0
, .r_roll_acc = 1.0
, .preview = {
.enabled = false
, .preview_window_name = "aimer_preview"
, .preview_scale = 0.5
, .preview_wait_key_ms = 1
, .queue_capacity = 1
, .output_mode = "window"
, .web_bind_address = "0.0.0.0"
, .web_port = 8080
, .web_stream_name = "aimer_preview"
, .max_fps = 30.0
}
, .enable_runtime_log = true
, .bullet_speed_log_delta = 0.05
, .heat_log_delta = 1.0
, .convert_raw_gimbal_quat_to_body = true
, .referee_topic = "robot_game_ref"
}
      , AutoAimRunConfig::MainCameraCalibration
  );
  static std::initializer_list<SharedTopicClient::TopicConfig> xr_arg_shared_topic_tx_topic_configs =
      {
{
"target_euler"
, "host"
}
, {
"fire_notify"
, "host"
}
, {
"camera_sync_command"
, "host"
}
}
  ;
  // modules[6]: shared_topic_tx
  static SharedTopicClient shared_topic_tx(
      static_cast<LibXR::UART&>(devc_usb)
      , 256
      , xr_arg_shared_topic_tx_topic_configs
  );
  static_assert(std::is_void_v<decltype(camera.OnMonitor())>, "camera.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(camera_frame_sync.OnMonitor())>, "camera_frame_sync.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(shared_topic_rx.OnMonitor())>, "shared_topic_rx.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(armor_detector.OnMonitor())>, "armor_detector.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(armor_tracker.OnMonitor())>, "armor_tracker.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(aimer.OnMonitor())>, "aimer.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(shared_topic_tx.OnMonitor())>, "shared_topic_tx.OnMonitor() must return void");
  for (;;) {
    camera.OnMonitor();
    camera_frame_sync.OnMonitor();
    shared_topic_rx.OnMonitor();
    armor_detector.OnMonitor();
    armor_tracker.OnMonitor();
    aimer.OnMonitor();
    shared_topic_tx.OnMonitor();
    LibXR::Thread::Sleep(1000);
  }
}

#undef XR_XROBOT_MAIN_INLINE

/* User Code Begin XRobotMain */
/* User Code End XRobotMain */
// clang-format off
// NOLINTBEGIN
#define XR_REGISTER_DETAIL_ramfs(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::RamFS>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(ramfs)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER_DETAIL_devc_usb(...) \
  static_assert(std::is_same<::xrobot_generated::TypeList<__VA_ARGS__>, \
      ::xrobot_generated::TypeList<LibXR::UART>>::value && \
      ::xrobot_generated::RegistrationMatches< \
          std::remove_reference_t<decltype(devc_usb)>, __VA_ARGS__>::value, \
      "XR_REGISTER changed; regenerate xrobot_main.hpp")
#define XR_REGISTER(name, ...) XR_REGISTER_DETAIL_##name(__VA_ARGS__)

#define XROBOT_MAIN() ::XRobotMain(ramfs, devc_usb)

// NOLINTEND
// clang-format on
