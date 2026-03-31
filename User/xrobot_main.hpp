#include "app_framework.hpp"
#include "libxr.hpp"

// Module headers
#include "CameraBase.hpp"
#include "DemoReplay.hpp"
#include "ArmorDetector.hpp"
#include "ReplayMetrics.hpp"
#include "ArmorTracker.hpp"
#include "Aimer.hpp"
#include "HikCamera.hpp"

static void XRobotMain(LibXR::HardwareContainer &hw) {
  using namespace LibXR;
  ApplicationManager appmgr;

  // Auto-generated module instantiations
  static HikCamera hik_camera(hw, appmgr, {1440, 1080, 4320, 0, CameraBase::Encoding::RGB8, {2340.46464112537, 0.0, 713.3224120377864, 0.0, 2336.8745144649124, 547.4106752074272, 0.0, 0.0, 1.0}, CameraBase::DistortionModel::PLUMB_BOB, {-0.09558691800515781, 0.3013704144837407, -0.0008218465102445683, 0.00024582434306615617, 0.0}, {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, {2323.906982421875, 0.0, 712.9446224841959, 0.0, 0.0, 2324.767578125, 546.6426169058832, 0.0, 0.0, 0.0, 1.0, 0.0}}, {32.0, 600.0, true});
  static ArmorDetector armor_detector(hw, appmgr, {1, {150.0, 45.0, 1.5, 20.0, 8.0, 1.0, 5.0, 1.5, 25.0}, {false, 420, 50, 600, 600, true, 0.7, 0.3, 0.8}, {false, false, 1, 0.75}});
  static ArmorTracker armor_tracker(hw, appmgr, {{10.0, 1.0}, {5, 15, 75}, {{0.4938528651012095, -0.5019457735940993, 0.49263726396521773, -0.5113397247967462}, {0.09496930183353451, 0.09500629029800668, 0.05098706629175661}}, {false, 1, 0.75, true}});
  static Aimer aimer(hw, appmgr, {0.0, 0.0, 60.0, 20.0, 2.0, 0.03, 0.015, 23.0, 14.0, 3.0, 2.0, 2.0, true});

  while (true) {
    appmgr.MonitorAll();
    Thread::Sleep(1000);
  }
}