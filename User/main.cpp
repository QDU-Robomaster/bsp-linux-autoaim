// Hardware entry: Hik camera plus the DevC USB link. Select the run config by
// generating User/xrobot_main.hpp from User/xrobot.yaml or User/RunConfig/*.yaml.

#include "Referee.hpp"
#include "bsp_common.hpp"
#include "linux_uart.hpp"
#include "run_config.hpp"
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

  // SharedTopic finds received topics by name; create the typed referee summary
  // before XRobotMain constructs the receiver.
  static LibXR::Topic::Domain host_domain("host");
  [[maybe_unused]] static LibXR::Topic robot_game_referee_topic =
      LibXR::Topic::CreateTopic<Referee::RobotGameRefereePack>("robot_game_ref",
                                                                &host_domain, true);

  XR_REGISTER(ramfs, LibXR::RamFS);
  XR_REGISTER(devc_usb, LibXR::UART);
  XROBOT_MAIN();
}
