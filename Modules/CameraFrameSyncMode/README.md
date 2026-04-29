# CameraFrameSyncMode

极小的 xrobot 配置模块，用于在 BSP preset 中设置 `CameraFrameSync` 模式。

用途：

- `capturefile.yaml`：设置为 `CameraFrameSync::SyncMode::LATEST_IMU`
- `hik.yaml`：不使用本模块，保持 `CameraFrameSync` 默认 `RAW_PROBE`

这个模块不改变 `CameraFrameSync` 构造语义，只避免为了一个 preset 去改 sync 模块默认值。
