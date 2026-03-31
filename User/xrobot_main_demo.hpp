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
  static DemoReplay demo_replay(hw, appmgr, {"Video/demo.avi", "Video/demo.txt", 27.0, false, {1440, 1080, 4320, 0, CameraBase::Encoding::BGR8, {1818.3669452465165, 0.0, 751.062265747035, 0.0, 1822.494494078506, 530.4367155611213, 0.0, 0.0, 1.0}, CameraBase::DistortionModel::PLUMB_BOB, {-0.07794462659956886, 0.1544782603148689, -0.0025714394278524674, 0.0008301631130127363, 0.0}, {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, {1818.3669452465165, 0.0, 751.062265747035, 0.0, 0.0, 1822.494494078506, 530.4367155611213, 0.0, 0.0, 0.0, 1.0, 0.0}}});
  static ArmorDetector armor_detector(hw, appmgr, {1, {150.0, 45.0, 1.5, 20.0, 8.0, 1.0, 5.0, 1.5, 25.0}, {false, 420, 50, 600, 600, true, 0.7, 0.3, 0.8}, {false, false, 1, 0.75}});
  static ArmorTracker armor_tracker(hw, appmgr, {{10.0, 1.0}, {5, 15, 75}, {{0.4938528651012095, -0.5019457735940993, 0.49263726396521773, -0.5113397247967462}, {0.09496930183353451, 0.09500629029800668, 0.05098706629175661}}, {true, 1, 0.75, true}});
  static Aimer aimer(hw, appmgr, {-1.0, -1.4, 60.0, 20.0, 2.0, 0.03, 0.015, 23.0, 14.0, 3.0, 2.0, 2.0, true});
  static ReplayMetrics replay_metrics(hw, appmgr, {true});

  while (true) {
    appmgr.MonitorAll();
    Thread::Sleep(100);
  }
}