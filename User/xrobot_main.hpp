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
  static HikCamera<AutoAimRunConfig::MainFrameLayout> camera(
      hw,
      appmgr,
      AutoAimRunConfig::MainCameraCalibration,
      {"gimbal", "camera_image", "camera_imu", 16.0F, AutoAimRunConfig::HikExposureTimeUs, true, 249.0F, 100, 3, 2, 2, false}
  );
  static CameraFrameSync<
      AutoAimRunConfig::MainFrameLayout
  > camera_frame_sync(
      hw,
      appmgr,
      camera,
      {CameraFrameSync<AutoAimRunConfig::MainFrameLayout>::SyncMode::RAW_PROBE, AutoAimRunConfig::HikSyncOffsetUs, "host", "camera_sync_command", "camera_sync_result", 3, 1, 100.0F}
  );
  static SharedTopic shared_topic_rx(
      hw,
      appmgr,
      "DevC-USB",
      4096,
      {{"gimbal_gyro", "host"}, {"gimbal_accl", "host"}, {"gimbal_quat", "host"}, {"camera_sync_result", "host"}, {"robot_game_ref", "host"}}
  );
  static ArmorDetector<AutoAimRunConfig::MainFrameLayout> armor_detector(
      hw,
      appmgr,
      {2, {ArmorDetectorModel::INT16_HEAD_L, 0.1, true, 16.0}, true, "host", "robot_game_ref", {false, "armor_detector_preview", 0.5, 1, 1, "window", "0.0.0.0", 8080, "armor_detector", 30.0}},
      camera_frame_sync
  );
  static ArmorTracker<AutoAimRunConfig::MainFrameLayout> armor_tracker(
      hw,
      appmgr,
      {{false, -1, 2, 15, 75, {1.6, 2.0, 1.2, 0.8, 2.0, 8.0, 7.5, 6000.0, 4.0, 8.0, 0.5, 0.55, 0.35, 0.25}}, {{{0.999929746909, -0.009747410407, -0.004716638126, -0.00482105397}, {0.04186176466382783, 0.136068364765315, 0.008995665883635868}}}, {false, "armor_tracker_preview", 0.5, 1, 1, "window", "0.0.0.0", 8080, "armor_tracker", 30.0}},
      camera_frame_sync
  );
  static Aimer<AutoAimRunConfig::MainFrameLayout> aimer(
      hw,
      appmgr,
      {0.0, 0.6, 2.0, 21.7, 14.0, 0.02, 0.001, 16, -20.0, 35.0, false, 0.0, 0.0, 0.0, 0.0, 0.05, 0.075, 0.075, 0.06, 0.48, true, 0.48, 50.0, 9000000.0, 0.0, 1.0, 100.0, 9000000.0, 0.0, 1.0, {false, "aimer_preview", 0.5, 1, 1, "window", "0.0.0.0", 8080, "aimer_preview", 30.0}, true, 0.05, 1.0, true},
      AutoAimRunConfig::MainCameraCalibration
  );
  static SharedTopicClient shared_topic_tx(
      hw,
      appmgr,
      "DevC-USB",
      256,
      {{"camera_sync_command", "host"}}
  );

  while (true) {
    appmgr.MonitorAll();
    Thread::Sleep(1000);
  }
}