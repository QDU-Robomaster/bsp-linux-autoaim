#include "app_framework.hpp"
#include "libxr.hpp"

// Module headers
#include "HikCamera.hpp"
#include "CameraFrameSync.hpp"
#include "SharedTopic.hpp"
#include "ArmorDetector.hpp"
#include "ArmorTracker.hpp"
#include "Aimer.hpp"
#include "SharedTopicClient.hpp"
#include "xrobot_constexpr.hpp"

static void XRobotMain(LibXR::HardwareContainer &hw) {
  using namespace LibXR;
  ApplicationManager appmgr;

  // Auto-generated module instantiations
  static HikCamera<AutoAimRunConfig::HikCameraInfo> camera(
      hw,
      appmgr,
      {"gimbal", "camera_image", "camera_imu", 16.0F, AutoAimRunConfig::HikExposureTimeUs, true, 249.0F, 100, 3, 2, 2, false}
  );
  static CameraFrameSync<AutoAimRunConfig::HikCameraInfo> camera_frame_sync(
      hw,
      appmgr,
      camera,
      {CameraFrameSync<AutoAimRunConfig::HikCameraInfo>::SyncMode::RAW_PROBE, AutoAimRunConfig::HikSyncOffsetUs, "host", "camera_sync_command", "camera_sync_result", 3, 1, 100.0F}
  );
  static SharedTopic shared_topic_rx(
      hw,
      appmgr,
      "DevC-USB",
      4096,
      {{"gimbal_gyro", "host"}, {"gimbal_accl", "host"}, {"gimbal_quat", "host"}, {"camera_sync_result", "host"}, {"robot_game_ref", "host"}}
  );
  static ArmorDetector<AutoAimRunConfig::HikCameraInfo> armor_detector(
      hw,
      appmgr,
      {2, {0.1, true, 16.0, "AUTO_DETECT", "LATENCY"}, true, "host", "robot_game_ref", {false, "armor_detector_preview", 0.5, 1, 1, "window", "0.0.0.0", 8080, "armor_detector", 30.0}, {true, 0.2, 0.9, true}},
      camera_frame_sync
  );
  static ArmorTracker<AutoAimRunConfig::HikCameraInfo> armor_tracker(
      hw,
      appmgr,
      {{false, -1, 2, 15, 75, {1.6, 2.0, 1.2, 0.8, 2.0, 8.0, 7.5, 6000.0, 4.0, 8.0, 0.5, 0.55, 0.35, 0.25}}, {{{1.0, 0.0, 0.0, 0.0}, {0.0, 0.115, 0.05}}}, {false, "armor_tracker_preview", 0.5, 1, 1, "window", "0.0.0.0", 8080, "armor_tracker", 30.0}},
      camera_frame_sync
  );
  static Aimer<AutoAimRunConfig::HikCameraInfo> aimer(
      hw,
      appmgr,
      {-1.0, -0.40107045659157625, 2.0, 23.0, 14.0, 0.02, 0.001, 16, -20.0, 35.0, true, 0.0, 0.0, 0.0, 0.0, 0.075, 0.075, 0.075, 0.003, 0.05, true, 0.05, 50.0, 9000000.0, 0.0, 1.0, 100.0, 9000000.0, 0.0, 1.0, {false, "aimer_preview", 0.5, 1, 1, "window", "0.0.0.0", 8080, "aimer_preview", 30.0}, true, 0.05, 1.0}
  );
  static SharedTopicClient shared_topic_tx(
      hw,
      appmgr,
      "DevC-USB",
      256,
      {{"target_euler", "host"}, {"fire_notify", "host"}, {"camera_sync_command", "host"}}
  );

  while (true) {
    appmgr.MonitorAll();
    Thread::Sleep(1000);
  }
}