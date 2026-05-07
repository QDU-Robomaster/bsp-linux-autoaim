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
  static HikCamera<AutoAimRunConfig::HikCameraInfo> camera(
      hw,
      appmgr,
      {"gimbal", "camera_image", "camera_imu", 16.0F, 2000.0F, true, 249.0F, 100, 3, 2, 2, false}
  );
  static CameraFrameSync<AutoAimRunConfig::HikCameraInfo> camera_frame_sync(
      hw,
      appmgr,
      camera,
      {CameraFrameSync<AutoAimRunConfig::HikCameraInfo>::SyncMode::RAW_PROBE, 0, "host", "camera_sync_command", "camera_sync_result", 3, 1, 100.0F}
  );
  static SharedTopic shared_topic_rx(
      hw,
      appmgr,
      "DevC-USB",
      4096,
      {{"gimbal_gyro", "host"}, {"gimbal_accl", "host"}, {"gimbal_quat", "host"}, {"camera_sync_result", "host"}, {"robot_game_ref", "host"}}
  );
  static SharedTopicClient shared_topic_tx(
      hw,
      appmgr,
      "DevC-USB",
      256,
      {{"camera_sync_command", "host"}}
  );
  static ArmorDetector<AutoAimRunConfig::HikCameraInfo> armor_detector(
      hw,
      appmgr,
      {2, {0.1, 0.1, true, 16.0, 512, 384, "AUTO_DETECT", "LATENCY"}, true, "host", "robot_game_ref", {false, "armor_detector_preview", 0.5, 1, 1, "window", "0.0.0.0", 8080, "armor_detector", 30.0}},
      camera_frame_sync
  );
  static ArmorTracker<AutoAimRunConfig::HikCameraInfo> armor_tracker(
      hw,
      appmgr,
      {{30.0, 30.0}, {0.15, 1.0}, {5, 0.3}, {20.0, 100.0, 800.0}, {0.26, 0.12, 0.4}, {0.05, 0.02}, {{0.49032232209180826, -0.5047863708428628, 0.5048907866866026, -0.4998600141927461}, {0.136068364765315, -0.04186176466382783, 0.008995665883635868}}, {true, 0.25, true, false, false, true, false}, {false, "armor_tracker_preview", 0.5, 1, 1, "window", "0.0.0.0", 8080, "armor_tracker", 30.0}},
      camera_frame_sync
  );

  while (true) {
    appmgr.MonitorAll();
    Thread::Sleep(1000);
  }
}