#pragma once

// Compile-time constants for each run configuration. Each YAML under
// User/ names its namespace explicitly; edit values here, not in generated code.

#include "CameraBase.hpp"

namespace AutoAimRunConfig {

namespace Hik {
inline constexpr float HikExposureTimeUs = 2000.0F;
inline constexpr int HikTriggerDelayUs = 75;
inline constexpr int HikSyncOffsetUs = HikTriggerDelayUs + static_cast<int>(HikExposureTimeUs * 0.5F);
inline constexpr CameraTypes::CameraCalibration MainCameraCalibration = {.native_width = 1440, .native_height = 1080, .camera_matrix = {2328.685719898089, 0.0, 733.3564625092474, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 1.0}, .distortion_model = CameraTypes::DistortionModel::PLUMB_BOB, .distortion_coefficients = {-0.09182103918709904, 0.4639907346830205, 0.002609878642637282, 0.0009819586010405485, -0.4751278850310457}, .rectification_matrix = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, .projection_matrix = {2328.685719898089, 0.0, 733.3564625092474, 0.0, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 0.0, 1.0, 0.0}};
inline constexpr CameraTypes::FrameLayout HikFrameLayout = {.width = 720, .height = 540, .step = 2160, .encoding = CameraTypes::Encoding::BGR8};
}  // namespace Hik

namespace CaptureFile {
inline constexpr CameraTypes::CameraCalibration MainCameraCalibration = {.native_width = 1440, .native_height = 1080, .camera_matrix = {2328.685719898089, 0.0, 733.3564625092474, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 1.0}, .distortion_model = CameraTypes::DistortionModel::PLUMB_BOB, .distortion_coefficients = {-0.09182103918709904, 0.4639907346830205, 0.002609878642637282, 0.0009819586010405485, -0.4751278850310457}, .rectification_matrix = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, .projection_matrix = {2328.685719898089, 0.0, 733.3564625092474, 0.0, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 0.0, 1.0, 0.0}};
inline constexpr CameraTypes::FrameLayout MainFrameLayout = {.width = 1440, .height = 1080, .step = 4320, .encoding = CameraTypes::Encoding::BGR8};
inline constexpr CameraTypes::FrameGeometry MainFrameGeometry = {.width = 1440, .height = 1080, .step = 4320, .roi_offset_x_native = 0, .roi_offset_y_native = 0, .decimation_x = 1, .decimation_y = 1, .flags = CameraTypes::FRAME_GEOMETRY_NONE, .reserved = 0, .sample_phase_x_native = 0.0F, .sample_phase_y_native = 0.0F};
}  // namespace CaptureFile

namespace Sentry {
inline constexpr float HikExposureTimeUs = 2000.0F;
inline constexpr int HikTriggerDelayUs = 75;
inline constexpr int HikSyncOffsetUs = HikTriggerDelayUs + static_cast<int>(HikExposureTimeUs * 0.5F);
inline constexpr CameraTypes::CameraCalibration MainCameraCalibration = {.native_width = 1440, .native_height = 1080, .camera_matrix = {2348.0610281828863, 0.0, 753.7199990513768, 0.0, 2341.430205137848, 544.3093638576938, 0.0, 0.0, 1.0}, .distortion_model = CameraTypes::DistortionModel::PLUMB_BOB, .distortion_coefficients = {-0.09324913488777978, 0.3089185338125273, 0.0011528970103605383, -0.0010514494107999794, 0.0}, .rectification_matrix = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, .projection_matrix = {2334.0852301109603, 0.0, 753.2190261545588, 0.0, 0.0, 2331.8191930180155, 544.7774518626655, 0.0, 0.0, 0.0, 1.0, 0.0}};
inline constexpr CameraTypes::FrameLayout HikFrameLayout = {.width = 720, .height = 540, .step = 2160, .encoding = CameraTypes::Encoding::BGR8};
}  // namespace Sentry

namespace VisionCapture {
inline constexpr float HikExposureTimeUs = 2000.0F;
inline constexpr int HikTriggerDelayUs = 75;
inline constexpr int HikSyncOffsetUs = HikTriggerDelayUs + static_cast<int>(HikExposureTimeUs * 0.5F);
inline constexpr CameraTypes::CameraCalibration MainCameraCalibration = {.native_width = 1440, .native_height = 1080, .camera_matrix = {2328.685719898089, 0.0, 733.3564625092474, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 1.0}, .distortion_model = CameraTypes::DistortionModel::PLUMB_BOB, .distortion_coefficients = {-0.09182103918709904, 0.4639907346830205, 0.002609878642637282, 0.0009819586010405485, -0.4751278850310457}, .rectification_matrix = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, .projection_matrix = {2328.685719898089, 0.0, 733.3564625092474, 0.0, 0.0, 2328.670107789996, 540.6187286922773, 0.0, 0.0, 0.0, 1.0, 0.0}};
inline constexpr CameraTypes::FrameLayout HikFrameLayout = {.width = 720, .height = 540, .step = 2160, .encoding = CameraTypes::Encoding::BGR8};
}  // namespace VisionCapture

}  // namespace AutoAimRunConfig
