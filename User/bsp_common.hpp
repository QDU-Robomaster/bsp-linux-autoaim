#pragma once

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "libxr.hpp"
#include "logger.hpp"
#include "message.hpp"
#include "ramfs.hpp"
#include "terminal.hpp"
#include "thread.hpp"

// Process setup of the autoaim executable: platform, single-instance lock, terminal, file log.
namespace AutoAimBsp
{
inline const char *FileLogLevelName(LibXR::LogLevel level)
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

inline int AcquireBspLock()
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

inline void WriteLogFile(bool, LibXR::Topic, LibXR::MicrosecondTimestamp timestamp,
                         const LibXR::LogData &log)
{
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
      // The file is named after the first log time: YYYYMMDD_HHMMSS.log
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
}

/**
 * @brief 初始化平台、单实例锁、终端线程和文件日志。
 *        Initialize the platform, single-instance lock, terminal and file log.
 *
 * @param ramfs 终端使用的 RamFS。
 *              RamFS used by the terminal.
 * @return 单实例锁已被其他 autoaim 进程持有时返回 false，否则返回 true。
 *         false when another autoaim process already holds the lock, otherwise true.
 */
inline bool Init(LibXR::RamFS &ramfs)
{
  LibXR::PlatformInit();

  if (AcquireBspLock() < 0)
  {
    return false;
  }

  XR_LOG_PASS("Platform initialized");

  using TerminalType = LibXR::Terminal<1024, 64, 16, 128>;
  static TerminalType terminal(ramfs);
  static LibXR::Thread term_thread;
  term_thread.Create(&terminal, TerminalType::ThreadFun, "terminal", 65536,
                     LibXR::Thread::Priority::MEDIUM);

  static auto log_topic = LibXR::Topic(LibXR::Topic::Find("/xr/log"));
  static auto log_cb = LibXR::Topic::Callback::Create(&WriteLogFile, log_topic);
  log_topic.RegisterCallback(log_cb);
  return true;
}
}  // namespace AutoAimBsp
