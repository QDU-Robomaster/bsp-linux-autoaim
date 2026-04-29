#pragma once

#include "CameraBase.hpp"

namespace AutoAimRunConfig {
inline constexpr CameraTypes::CameraInfo MainCameraInfo = CameraTypes::CameraInfo{1440, 1080, 4320, CameraTypes::Encoding::BGR8, {2328.6857198980888, 0.0, 733.35646250924742, 0.0, 2328.6701077899961, 540.61872869227727, 0.0, 0.0, 1.0}, CameraTypes::DistortionModel::PLUMB_BOB, {-0.091821039187099038, 0.46399073468302049, 0.0026098786426372819, 0.0009819586010405485, -0.47512788503104569}, {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, {2328.6857198980888, 0.0, 733.35646250924742, 0.0, 0.0, 2328.6701077899961, 540.61872869227727, 0.0, 0.0, 0.0, 1.0, 0.0}};
}  // namespace AutoAimRunConfig
