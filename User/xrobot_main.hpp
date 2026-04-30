#include "app_framework.hpp"
#include "libxr.hpp"

// Module headers
#include "HikCamera.hpp"
#include "CameraFrameSync.hpp"
#include "ArmorDetector.hpp"
#include "ArmorTracker.hpp"
#include "VisionPreview.hpp"
#include "xrobot_constexpr.hpp"

static void XRobotMain(LibXR::HardwareContainer &hw) {
  using namespace LibXR;
  ApplicationManager appmgr;

  // Auto-generated module instantiations
  static HikCamera<AutoAimRunConfig::MainCameraInfo> camera(
      hw,
      appmgr,
      HikCamera<AutoAimRunConfig::MainCameraInfo>::RuntimeParam{.camera_name = "camera", .image_topic_name = "camera_image", .imu_topic_name = "camera_imu", .gain = 32.0F, .exposure_time = 600.0F, .external_trigger = true, .acquisition_frame_rate = 249.0F, .grab_timeout_ms = 100, .image_node_num = 3}
  );
  static CameraFrameSync<
      AutoAimRunConfig::MainCameraInfo
  > camera_frame_sync(
      hw,
      appmgr,
      camera,
      CameraFrameSync<AutoAimRunConfig::MainCameraInfo>::RuntimeParam{.mode = CameraFrameSync<AutoAimRunConfig::MainCameraInfo>::SyncMode::RAW_PROBE, .offset_us = 0}
  );
  static ArmorDetector<AutoAimRunConfig::MainCameraInfo> armor_detector(
      hw,
      appmgr,
      ArmorDetector<AutoAimRunConfig::MainCameraInfo>::Config{.detect_color = 2, .traditional = {}, .yolo = {.use_roi = false, .roi_x = 420, .roi_y = 50, .roi_width = 600, .roi_height = 600, .use_traditional_refine = true, .score_threshold = 0.55, .nms_threshold = 0.30, .min_confidence = 0.55}},
      camera_frame_sync
  );
  static ArmorTracker<AutoAimRunConfig::MainCameraInfo> armor_tracker(
      hw,
      appmgr,
      ArmorTracker<AutoAimRunConfig::MainCameraInfo>::Config{.limits = {.max_armor_distance = 30.0, .max_z_position = 30.0}, .match = {.max_match_distance = 0.15, .max_match_yaw_diff = 0.50}, .thresholds = {.tracking_thres = 5, .lost_time_thres = 0.3}, .solver = {.k = 0.092, .bias_time = 100, .s_bias = 0.19133, .z_bias = 0.21265, .calculate_mode = SolveTrajectory::NORMAL, .table_config = TrajectoryTable::TableConfig(13.0, 0.0, 1.0, -1.0, 0.01, "table.bin")}, .ekf = {.sigma2_q_xyz = 20.0, .sigma2_q_yaw = 100.0, .sigma2_q_r = 800.0}, .geometry = {.initial_radius = 0.26, .min_radius = 0.12, .max_radius = 0.4}, .noise = {.r_xyz_factor = 0.05, .r_yaw = 0.02}, .frames = {.rotation = {0.49032232209180826, -0.5047863708428628, 0.5048907866866026, -0.4998600141927461}, .translation = {0.136068364765315, -0.041861764663827829, 0.0089956658836358675}}, .sp = {.enable_pair_dz = false, .measurement_recenter_alpha = 0.20, .quality_recenter = true}},
      camera_frame_sync
  );
  static VisionPreview<AutoAimRunConfig::MainCameraInfo> vision_preview(
      hw,
      appmgr,
      VisionPreview<AutoAimRunConfig::MainCameraInfo>::RuntimeParam{.enabled = true, .record_raw = true, .realtime_preview = false, .overlay = {.detector = true, .tracker = true, .candidate_debug = true}, .output_dir = "/tmp/autoaim_preview_hik", .raw_video_name = "raw.avi", .preview_window_name = "autoaim_preview", .preview_scale = 0.5, .preview_wait_key_ms = 1, .record_fps = 100.0},
      camera_frame_sync
  );

  while (true) {
    appmgr.MonitorAll();
    Thread::Sleep(1000);
  }
}
