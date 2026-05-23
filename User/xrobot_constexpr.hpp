#pragma once

#include "CameraBase.hpp"

namespace AutoAimRunConfig {
inline constexpr bool EnableDevCUsb = true;
inline constexpr float HikExposureTimeUs = 2000.0F;
inline constexpr int HikTriggerDelayUs = 75;
inline constexpr int HikSyncOffsetUs = HikTriggerDelayUs + static_cast<int>(HikExposureTimeUs * 0.5F);
inline constexpr CameraTypes::CameraInfo MainCameraInfo = {1440, 1080, 4320, CameraTypes::Encoding::BGR8, {2348.0610281828863, 0.0, 753.7199990513768, 0.0, 2341.430205137848, 544.3093638576938, 0.0, 0.0, 1.0}, CameraTypes::DistortionModel::PLUMB_BOB, {-0.09324913488777978, 0.3089185338125273, 0.0011528970103605383, -0.0010514494107999794, 0.0}, {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, {2334.0852301109603, 0.0, 753.2190261545588, 0.0, 0.0, 2331.8191930180155, 544.7774518626655, 0.0, 0.0, 0.0, 1.0, 0.0}};
inline constexpr CameraTypes::CameraInfo HikCameraInfo = {720, 540, 2160, CameraTypes::Encoding::BGR8, {1174.0305140914431, 0.0, 376.8599995256884, 0.0, 1170.715102568924, 272.1546819288469, 0.0, 0.0, 1.0}, CameraTypes::DistortionModel::PLUMB_BOB, {-0.09324913488777978, 0.3089185338125273, 0.0011528970103605383, -0.0010514494107999794, 0.0}, {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, {1167.0426150554802, 0.0, 376.6095130772794, 0.0, 0.0, 1165.9095965090078, 272.3887259313328, 0.0, 0.0, 0.0, 1.0, 0.0}};
}  // namespace AutoAimRunConfig
