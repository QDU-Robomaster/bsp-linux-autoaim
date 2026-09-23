// Replay entry: recorded frames only, no DevC USB link. Generated from
// User/Replay/xrobot.yaml into User/Replay/xrobot_main.hpp.

#include "bsp_common.hpp"
#include "run_config.hpp"
#include "xrobot_main.hpp"

int main(int, char **)
{
  static LibXR::RamFS ramfs;
  if (!AutoAimBsp::Init(ramfs))
  {
    return 1;
  }

  XR_REGISTER(ramfs, LibXR::RamFS);
  XROBOT_MAIN();
}
