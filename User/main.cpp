// 车上入口：海康相机 + DevC 自瞄口。产品按配置选择：
// `xrobot gen -c User/xrobot.yaml` 或 `xrobot gen -c User/RunConfig/<name>.yaml`。
// 单实例与重启由 systemd 负责，日志写到标准输出（systemd 下进 journald）。
//
// Robot entry: Hik camera plus the DevC auto-aim port. The product is chosen by
// configuration (`xrobot gen -c ...`). systemd keeps a single instance and restarts it;
// logs go to standard output (journald under systemd).

#include "RefereeTypes.hpp"
#include "libxr.hpp"
#include "linux_uart.hpp"
#include "ramfs.hpp"
#include "xrobot_main.hpp"

/// DevC 1.0 的 USB VID:PID 与自瞄 CDC 口的接口名 / DevC 1.0 USB IDs and the auto-aim
/// CDC interface name.
static constexpr const char* DEVC_VID = "1d50";
static constexpr const char* DEVC_PID = "6199";
static constexpr const char* DEVC_AUTOAIM_INTERFACE = "XRobot AutoAim";

int main(int, char**)
{
  LibXR::PlatformInit();

  static LibXR::RamFS ramfs;
  static LibXR::LinuxUART devc_usb(DEVC_VID, DEVC_PID, DEVC_AUTOAIM_INTERFACE, 115200,
                                   LibXR::UART::Parity::NO_PARITY, 8, 1, 80, 8192);

  // SharedTopic 按名字查找要转发的 Topic，带类型的裁判摘要 Topic 必须在它构造前存在。
  // 这是入口唯一引用模块类型的地方（见 README）。
  // SharedTopic looks received Topics up by name, so the typed referee summary Topic
  // must exist before it is constructed; the one place where the entry names a Module
  // type (see README).
  static LibXR::Topic::Domain host_domain("host");
  [[maybe_unused]] static LibXR::Topic robot_game_referee_topic =
      LibXR::Topic::CreateTopic<RefereeTypes::RobotGameRefereePack>(
          "robot_game_ref", &host_domain, true);

  XR_REGISTER(ramfs, LibXR::RamFS);
  XR_REGISTER(devc_usb, LibXR::UART);
  XROBOT_MAIN();
}
