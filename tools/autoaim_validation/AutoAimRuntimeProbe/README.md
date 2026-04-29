# AutoAimRuntimeProbe

验证专用 topic 计数模块。

它会订阅：

- raw IMU：`camera_gyro`、`camera_accl`、`camera_quat`
- synced IMU：`camera_imu`
- detector domain：`armors_frame`、`metrics`
- tracker domain：`target`、`candidate_debug`

注意：

- 不放进正式 `capturefile.yaml` 或 `hik.yaml`。
- `scripts/validate_runconfigs_ubuntu24.sh` 会在临时 BSP 树中生成
  `capturefile_probe.yaml`，只在验证运行时插入本模块。
