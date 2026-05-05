#pragma once

#include "CameraBase.hpp"

namespace AutoAimRunConfig {
inline constexpr bool EnableDevCUsb = true;
inline constexpr CameraTypes::CameraInfo MainCameraInfo = {1440, 1080, 4320, CameraTypes::Encoding::BGR8, {2328.685719898089, 0.0, 733.3564625092474, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 1.0}, CameraTypes::DistortionModel::PLUMB_BOB, {-0.09182103918709904, 0.4639907346830205, 0.002609878642637282, 0.0009819586010405485, -0.4751278850310457}, {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, {2328.685719898089, 0.0, 733.3564625092474, 0.0, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 0.0, 1.0, 0.0}};
}  // namespace AutoAimRunConfig
