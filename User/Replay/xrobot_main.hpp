#pragma once
// xrobot-stamp: config=xrobot.yaml sha256=322e065d72c3ccd22817df2cff5abb3445f43a2b6ce6d53da6a6785c50894032
// xrobot-stamp: lock=../../xrobot.lock sha256=ec47458d4271263753e102adb37c3cb326c07b32092bdbf279b1c9db2b589f7d
// xrobot-stamp: tool=xrobot 0.3.1

#include <memory>
#include <type_traits>
#include <utility>
#include "libxr.hpp"
#include "thread.hpp"
#include "CaptureFileCamera.hpp"
#include "CameraFrameSync.hpp"
#include "ArmorDetector.hpp"
#include "ArmorTracker.hpp"
#include "Aimer.hpp"
#include "CameraBase.hpp"

namespace xrobot_generated {
template <typename...> struct TypeList {};
template <typename Source, typename... Views>
struct RegistrationMatches
    : std::bool_constant<(!std::is_reference<Views>::value && ...) &&
                         (std::is_convertible<Source*, Views*>::value && ...)> {};

}  // namespace xrobot_generated

namespace AutoAimRunConfig {
inline constexpr CameraTypes::CameraCalibration MainCameraCalibration = {.native_width = 1440, .native_height = 1080, .camera_matrix = {2328.685719898089, 0.0, 733.3564625092474, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 1.0}, .distortion_model = CameraTypes::DistortionModel::PLUMB_BOB, .distortion_coefficients = {-0.09182103918709904, 0.4639907346830205, 0.002609878642637282, 0.0009819586010405485, -0.4751278850310457}, .rectification_matrix = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, .projection_matrix = {2328.685719898089, 0.0, 733.3564625092474, 0.0, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 0.0, 1.0, 0.0}};
inline constexpr CameraTypes::FrameLayout MainFrameLayout = {.width = 1440, .height = 1080, .step = 4320, .encoding = CameraTypes::Encoding::BGR8};
inline constexpr CameraTypes::FrameGeometry MainFrameGeometry = {.width = 1440, .height = 1080, .step = 4320, .roi_offset_x_native = 0, .roi_offset_y_native = 0, .decimation_x = 1, .decimation_y = 1, .flags = CameraTypes::FRAME_GEOMETRY_NONE, .reserved = 0, .sample_phase_x_native = 0.0F, .sample_phase_y_native = 0.0F};
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
    LibXR::RamFS& ramfs) {
  // modules[0]: camera
  static CaptureFileCamera<AutoAimRunConfig::MainFrameLayout> camera(
      static_cast<LibXR::RamFS&>(ramfs)
      , AutoAimRunConfig::MainCameraCalibration
      , CaptureFileCamera<AutoAimRunConfig::MainFrameLayout>::RuntimeParam(
static_cast<std::string_view>("./data/camera_internal_recording_20260428/damo_clean.avi")
, static_cast<std::string_view>("")
, static_cast<std::string_view>("./data/camera_internal_recording_20260428/damo_imu.csv")
, static_cast<std::string_view>("capturefile_camera")
, static_cast<std::string_view>("capturefile_image")
, static_cast<std::string_view>("capturefile_imu")
, static_cast<bool>(true)
, static_cast<bool>(false)
, static_cast<uint32_t>(0)
, static_cast<CaptureFileCamera<AutoAimRunConfig::MainFrameLayout>::FrameGeometry>(AutoAimRunConfig::MainFrameGeometry)
, static_cast<double>(1.0)
)
  );
  // modules[1]: camera_frame_sync
  static CameraFrameSync<AutoAimRunConfig::MainFrameLayout> camera_frame_sync(
      static_cast<CameraFrameSync<AutoAimRunConfig::MainFrameLayout>::Base&>(camera)
      , CameraFrameSync<AutoAimRunConfig::MainFrameLayout>::RuntimeParam(
static_cast<CameraFrameSync<AutoAimRunConfig::MainFrameLayout>::SyncMode>(CameraFrameSync<AutoAimRunConfig::MainFrameLayout>::SyncMode::LATEST_IMU)
, static_cast<int32_t>(0)
, static_cast<std::string_view>("libxr_def_domain")
, static_cast<std::string_view>("camera_sync_command")
, static_cast<std::string_view>("camera_sync_result")
, static_cast<uint32_t>(3)
, static_cast<uint32_t>(1)
, static_cast<float>(50.0F)
, static_cast<CameraFrameSync<AutoAimRunConfig::MainFrameLayout>::RawImuFrame>(CameraFrameSync<AutoAimRunConfig::MainFrameLayout>::RawImuFrame::BODY_X_RIGHT_Y_FORWARD_Z_UP)
, std::remove_cv_t<std::remove_reference_t<std::string_view>>{}
)
  );
  // modules[2]: armor_detector
  static ArmorDetector<AutoAimRunConfig::MainFrameLayout> armor_detector(
      static_cast<ArmorDetector<AutoAimRunConfig::MainFrameLayout>::Sync&>(camera_frame_sync)
      , ArmorDetector<AutoAimRunConfig::MainFrameLayout>::Config{
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
, .referee_auto_detect_color = false
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
  // modules[3]: armor_tracker
  static ArmorTracker<AutoAimRunConfig::MainFrameLayout> armor_tracker(
      static_cast<LibXR::RamFS&>(ramfs)
      , static_cast<ArmorTracker<AutoAimRunConfig::MainFrameLayout>::FrameSync&>(camera_frame_sync)
      , ArmorTracker<AutoAimRunConfig::MainFrameLayout>::Config{
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
  // modules[4]: aimer
  static Aimer<AutoAimRunConfig::MainFrameLayout> aimer(
      Aimer<AutoAimRunConfig::MainFrameLayout>::Config{
.yaw_offset = -1.0
, .roll_offset = -1.4
, .yaw_rate_threshold = 2.0
, .default_bullet_speed = 23.0
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
, .fire_delay_s = 0.0
, .low_speed_extra_predict_s = 0.015
, .high_speed_extra_predict_s = 0.03
, .min_fire_threshold = 0.003
, .max_fire_threshold = 0.05
, .enable_mpc_plan = true
, .mpc_fire_thresh = 0.05
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
, .convert_raw_gimbal_quat_to_body = false
, .referee_topic = "robot_game_ref"
}
      , AutoAimRunConfig::MainCameraCalibration
  );
  static_assert(std::is_void_v<decltype(camera.OnMonitor())>, "camera.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(camera_frame_sync.OnMonitor())>, "camera_frame_sync.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(armor_detector.OnMonitor())>, "armor_detector.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(armor_tracker.OnMonitor())>, "armor_tracker.OnMonitor() must return void");
  static_assert(std::is_void_v<decltype(aimer.OnMonitor())>, "aimer.OnMonitor() must return void");
  for (;;) {
    camera.OnMonitor();
    camera_frame_sync.OnMonitor();
    armor_detector.OnMonitor();
    armor_tracker.OnMonitor();
    aimer.OnMonitor();
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
#define XR_REGISTER(name, ...) XR_REGISTER_DETAIL_##name(__VA_ARGS__)

#define XROBOT_MAIN() ::XRobotMain(ramfs)

// NOLINTEND
// clang-format on
