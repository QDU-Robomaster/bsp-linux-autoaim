// Hardware entry: Hik camera plus the DevC USB link. Select the product with
// `xrobot gen -c User/xrobot.yaml` or `xrobot gen -c User/RunConfig/<name>.yaml`.

#include "RefereeTypes.hpp"
#include "bsp_common.hpp"
#include "linux_uart.hpp"
#include "xrobot_main.hpp"

int main(int, char **)
{
  static LibXR::RamFS ramfs;
  if (!AutoAimBsp::Init(ramfs))
  {
    return 1;
  }

  static LibXR::LinuxUART devc_usb("16d0", "1492", 115200, LibXR::UART::Parity::NO_PARITY,
                                   8, 1, 80, 8192);

  // SharedTopic only looks received topics up by name, so the typed referee
  // summary topic must exist before XRobotMain constructs the receiver. This is
  // the one place where the entry names a Module type (see README).
  static LibXR::Topic::Domain host_domain("host");
  [[maybe_unused]] static LibXR::Topic robot_game_referee_topic =
      LibXR::Topic::CreateTopic<RefereeTypes::RobotGameRefereePack>(
          "robot_game_ref", &host_domain, true);

  XR_REGISTER(ramfs, LibXR::RamFS);
  XR_REGISTER(devc_usb, LibXR::UART);
  XROBOT_MAIN();
}
