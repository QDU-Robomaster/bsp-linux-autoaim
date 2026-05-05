# Linux AutoAim Run Config

这个目录放 `xrobot` 运行配置。仓库只保留一份生成后的
`User/xrobot_main.hpp`，切换配置后需要重新生成。

## Files

- `hik.yaml`: 实机配置。
- `capturefile.yaml`: 离线文件配置。

## Generate

```bash
python3 -m xrobot.GenerateMain --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/hik.yaml --output User/xrobot_main.hpp
python3 -m xrobot.GenerateMain --config User/RunConfig/capturefile.yaml --output User/xrobot_main.hpp
```

不带 `--config` 时使用 `User/xrobot.yaml`。
