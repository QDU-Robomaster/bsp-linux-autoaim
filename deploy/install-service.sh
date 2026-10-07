#!/usr/bin/env bash
# 安装并启动 systemd 服务：开机自启、退出后 2 秒重启、日志进 journald。
# 先按 README 构建好 build/release/rm_auto_aim，再在仓库根目录运行：
#   sudo deploy/install-service.sh            # 以调用 sudo 的用户运行
#   sudo deploy/install-service.sh --remove   # 停止并删除服务
#
# Install and start the systemd service: starts at boot, restarts 2 s after an exit,
# logs to journald. Build build/release/rm_auto_aim as in the README, then run this from the
# repository root (see above).
set -euo pipefail

unit=rm-auto-aim.service
target=/etc/systemd/system/$unit

if [ "$(id -u)" -ne 0 ]; then
  echo "run with sudo" >&2
  exit 1
fi

if [ "${1:-}" = "--remove" ]; then
  systemctl disable --now "$unit" 2>/dev/null || true
  rm -f "$target"
  systemctl daemon-reload
  echo "removed $unit"
  exit 0
fi

root=$(cd "$(dirname "$0")/.." && pwd)
user=${SUDO_USER:-$(id -un)}
if [ ! -x "$root/build/release/rm_auto_aim" ]; then
  echo "$root/build/release/rm_auto_aim not found; build it first (see README)" >&2
  exit 1
fi

sed -e "s#@ROOT@#$root#g" -e "s#@USER@#$user#g" "$root/deploy/$unit" > "$target"
systemctl daemon-reload
systemctl enable --now "$unit"
echo "installed $unit for $user in $root; logs: journalctl -u $unit -f"
