#include "app_framework.hpp"
#include "libxr.hpp"

// Module headers
#include "HikCamera.hpp"
#include "CameraFrameSync.hpp"
#include "SharedTopic.hpp"
#include "SharedTopicClient.hpp"
#include "ArmorDetector.hpp"
#include "ArmorTracker.hpp"
#include "xrobot_constexpr.hpp"

static void XRobotMain(LibXR::HardwareContainer &hw) {
  using namespace LibXR;
  ApplicationManager appmgr;

  // Auto-generated module instantiations
  static HikCamera<AutoAimRunConfig::MainCameraInfo> camera(
      hw,
      appmgr,
      HikCamera<AutoAimRunConfig::MainCameraInfo>::RuntimeParam{.camera_name = "gimbal", .image_topic_name = "camera_image", .imu_topic_name = "camera_imu", .gain = 15.0F, .exposure_time = 600.0F, .external_trigger = true, .acquisition_frame_rate = 249.0F, .grab_timeout_ms = 100, .image_node_num = 3}
  );
  static CameraFrameSync<
      AutoAimRunConfig::MainCameraInfo
  > camera_frame_sync(
      hw,
      appmgr,
      camera,
      CameraFrameSync<AutoAimRunConfig::MainCameraInfo>::RuntimeParam{.mode = CameraFrameSync<AutoAimRunConfig::MainCameraInfo>::SyncMode::RAW_PROBE, .offset_us = 0}
  );
  static SharedTopic shared_topic_rx(
      hw,
      appmgr,
      "DevC-USB",
      256,
      {SharedTopic::TopicConfig{"gimbal_gyro", "host"}, SharedTopic::TopicConfig{"gimbal_accl", "host"}, SharedTopic::TopicConfig{"gimbal_quat", "host"}, SharedTopic::TopicConfig{"camera_sync_result", "host"}}
  );
  static SharedTopicClient shared_topic_tx(
      hw,
      appmgr,
      "DevC-USB",
      16,
      {SharedTopicClient::TopicConfig{"camera_sync_command", "host"}}
  );
  static ArmorDetector<AutoAimRunConfig::MainCameraInfo> armor_detector(
      hw,
      appmgr,
      ArmorDetector<AutoAimRunConfig::MainCameraInfo>::Config{.detect_color = 2, .network = {.score_threshold = 0.1, .min_confidence = 0.1, .enable_quad_check = true, .min_quad_area_px = 16.0}},
      camera_frame_sync
  );
  static ArmorTracker<AutoAimRunConfig::MainCameraInfo> armor_tracker(
      hw,
      appmgr,
      ArmorTracker<AutoAimRunConfig::MainCameraInfo>::Config{.limits = {.max_armor_distance = 30.0, .max_z_position = 30.0}, .match = {.max_match_distance = 0.15, .max_match_yaw_diff = 1.0}, .thresholds = {.tracking_thres = 5, .lost_time_thres = 0.3}, .solver = {.k = 0.092, .bias_time = 100, .s_bias = 0.19133, .z_bias = 0.21265, .calculate_mode = SolveTrajectory::NORMAL, .table_config = TrajectoryTable::TableConfig(13.0, 0.0, 1.0, -1.0, 0.01, "table.bin")}, .ekf = {.sigma2_q_xyz = 20.0, .sigma2_q_yaw = 100.0, .sigma2_q_r = 800.0}, .geometry = {.initial_radius = 0.26, .min_radius = 0.12, .max_radius = 0.4}, .noise = {.r_xyz_factor = 0.05, .r_yaw = 0.02}, .frames = {.rotation = {0.49032232209180826, -0.5047863708428628, 0.5048907866866026, -0.4998600141927461}, .translation = {0.136068364765315, -0.041861764663827829, 0.0089956658836358675}}},
      camera_frame_sync
  );

  while (true) {
    appmgr.MonitorAll();
    Thread::Sleep(1000);
  }
}
