#include <chrono>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include "Aimer.hpp"
#include "app_framework.hpp"
#include "libxr.hpp"
#include "libxr_rw.hpp"
#include "libxr_system.hpp"
#include "linux_uart.hpp"
#include "logger.hpp"
#include "message.hpp"
#include "ramfs.hpp"
#include "terminal.hpp"
#include "thread.hpp"
#include "xrobot_constexpr.hpp"
#include "xrobot_main.hpp"

namespace
{
const char *FileLogLevelName(LibXR::LogLevel level)
{
  switch (level)
  {
    case LibXR::LogLevel::XR_LOG_LEVEL_ERROR:
      return "E";
    case LibXR::LogLevel::XR_LOG_LEVEL_WARN:
      return "W";
    case LibXR::LogLevel::XR_LOG_LEVEL_PASS:
      return "P";
    case LibXR::LogLevel::XR_LOG_LEVEL_INFO:
      return "I";
    case LibXR::LogLevel::XR_LOG_LEVEL_DEBUG:
      return "D";
    default:
      return "?";
  }
}
}  // namespace

static int AcquireBspLock()
{
  constexpr const char *lock_path = "/tmp/xrobot-autoaim-camera.lock";
  int fd = open(lock_path, O_CREAT | O_RDWR | O_CLOEXEC, 0666);
  if (fd < 0)
  {
    XR_LOG_ERROR("failed to open BSP lock %s: %s", lock_path, std::strerror(errno));
    return -1;
  }

  if (flock(fd, LOCK_EX | LOCK_NB) != 0)
  {
    XR_LOG_ERROR("another autoaim BSP is already running (%s)", lock_path);
    close(fd);
    return -1;
  }
  return fd;
}

void (*log_cb_fun)(bool in_isr, LibXR::Topic,
                   const LibXR::Topic::MessageView<LibXR::LogData>& log_message) =
    [](bool, LibXR::Topic tp,
       const LibXR::Topic::MessageView<LibXR::LogData>& log_message)
{
  UNUSED(tp);
  ASSERT(log_message.data != nullptr);

  const auto& log = *log_message.data;
  const auto timestamp = log_message.timestamp;

  if (LibXR::STDIO::write_ && LibXR::STDIO::write_->Writable())
  {
    using clock = std::chrono::system_clock;

    static std::ofstream f;
    if (!f.is_open())
    {
      auto now = clock::now();
      std::time_t t = clock::to_time_t(now);
      std::tm tm{};
      localtime_r(&t, &tm);

      std::ostringstream oss;
      // 首次打开时按启动时间命名：YYYYMMDD_HHMMSS.log
      oss << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".log";
      f.open(oss.str(), std::ios::out | std::ios::app);

      LibXR::STDIO::Printf<"Log written to %s\n">(oss.str().c_str());
    }

    if (f)
    {
      const uint32_t timestamp_ms =
          static_cast<uint32_t>(static_cast<uint64_t>(timestamp) / 1000U);
      f << FileLogLevelName(log.level) << " [" << timestamp_ms << "]("
        << (log.file ? log.file : "?") << ':' << log.line << ") " << log.message
        << '\n';
      f.flush();
    }
  }
};

int main(int, char **)
{
  LibXR::PlatformInit();

  const int bsp_lock_fd = AcquireBspLock();
  if (bsp_lock_fd < 0)
  {
    return 1;
  }
  (void)bsp_lock_fd;

  XR_LOG_PASS("Platform initialized");

  LibXR::RamFS ramfs;

  LibXR::Terminal<1024, 64, 16, 128> terminal(ramfs);

  LibXR::Thread term_thread;
  term_thread.Create(&terminal, LibXR::Terminal<1024, 64, 16, 128>::ThreadFun, "terminal",
                     65536, LibXR::Thread::Priority::MEDIUM);

  auto log_topic = LibXR::Topic(LibXR::Topic::Find("/xr/log"));
  auto log_cb = LibXR::Topic::Callback::Create(log_cb_fun, log_topic);
  log_topic.RegisterCallback(log_cb);

  LibXR::HardwareContainer peripherals{
      LibXR::Entry<LibXR::RamFS>({ramfs, {"ramfs"}}),
  };

  if constexpr (AutoAimRunConfig::EnableDevCUsb)
  {
    static LibXR::LinuxUART devc_usb("16d0", "1492", 115200,
                                     LibXR::UART::Parity::NO_PARITY, 8, 1, 80, 8192);
    peripherals.Register(LibXR::Entry<LibXR::UART>({devc_usb, {"DevC-USB"}}));

    static LibXR::Topic::Domain host_domain("host");
    [[maybe_unused]] static LibXR::Topic robot_game_referee_topic =
        LibXR::Topic::CreateTopic<AimerRefereeSummary>("robot_game_ref", &host_domain,
                                                       true);
  }

  XRobotMain(peripherals);
  while (1)
  {
    LibXR::Thread::Sleep(1000);
  }
  return 0;
}
