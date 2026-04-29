# CaptureFileCamera

文件相机模块，用内录文件复现真实相机输入链路。

当前验证文件为 `/home/xiao/data/camera_internal_recording_20260428/damo.avi`：

- FFV1/BGRA，`1440x1080@100Hz`
- 每帧开头 40 字节为 little-endian `double time,w,x,y,z`
- 输出图像为 `BGR8 1440x1080 step=4320`
- 默认清零嵌入头，避免 detector 把 header 字节当成图像内容
- 发布 raw IMU topics：`camera_gyro`、`camera_accl`、`camera_quat`

限制：

- 录制文件没有加速度，`camera_accl` 当前发布零向量。
- 陀螺由相邻四元数差分估计，只用于支撑离线验证。
- 文件源使用 `CameraFrameSync::LATEST_IMU`，不模拟 Hik 实机的硬件触发延迟。
