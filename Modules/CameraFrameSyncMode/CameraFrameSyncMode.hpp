#pragma once

// clang-format off
/* === MODULE MANIFEST V2 ===
module_description: Configure CameraFrameSync mode from an xrobot YAML preset
constructor_args:
  sync: "@camera_frame_sync"
  mode: CameraFrameSync<Info>::SyncMode::LATEST_IMU
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
  - qdu-future/CameraFrameSync
=== END MANIFEST === */
// clang-format on

#include "CameraFrameSync.hpp"
#include "app_framework.hpp"

template <CameraTypes::CameraInfo CameraInfoV>
class CameraFrameSyncMode : public LibXR::Application
{
 public:
  using Sync = CameraFrameSync<CameraInfoV>;
  using SyncMode = typename Sync::SyncMode;

  CameraFrameSyncMode(LibXR::HardwareContainer&, LibXR::ApplicationManager& app,
                      Sync& sync, SyncMode mode)
      : sync_(sync)
  {
    sync_.SetSyncMode(mode);
    app.Register(*this);
  }

  void OnMonitor() override {}

 private:
  Sync& sync_;
};
