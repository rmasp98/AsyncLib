#ifndef ASYNC_LIB_LOGGER_HPP
#define ASYNC_LIB_LOGGER_HPP

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "AsyncLib/worker.hpp"
#include "fmt/format.h"

#ifndef LOG_LEVEL
#ifdef NDEBUG
#define LOG_LEVEL 2
#else
#define LOG_LEVEL 4
#endif
#endif

namespace async_lib {

typedef std::chrono::high_resolution_clock timer;

namespace internal {
enum class LogLevel { ERROR, WARN, INFO };
static std::unordered_map<LogLevel, char const*> LogLevelName{
    {LogLevel::ERROR, "Error"},
    {LogLevel::WARN, "Warning"},
    {LogLevel::INFO, "Info"}};

struct Log {
  std::string message;
  LogLevel level;
  std::string component;
  std::string format;
  float time;
};

}  // namespace internal

// TODO: Add colour to logs (supported in libfmt)
class Logger {
 public:
  Logger(std::string const& name,
         std::shared_ptr<Worker<const internal::Log>> const& worker)
      : name_(name), worker_(worker) {}

  ~Logger() = default;

  // TODO: implement these
  Logger(Logger const&) = delete;
  Logger& operator=(Logger const&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(Logger&&) = delete;

  // TODO: Look into std::forward

  template <typename... Args>
  void Error(char const* format, Args&&... args) {
#if LOG_LEVEL >= 2
    SendLog(internal::LogLevel::ERROR, format, args...);
#endif
  }

  template <typename... Args>
  void Warn(const char* format, Args&&... args) {
#if LOG_LEVEL >= 3
    SendLog(internal::LogLevel::WARN, format, args...);
#endif
  }

  template <typename... Args>
  void Info(const char* format, Args&&... args) {
#if LOG_LEVEL >= 4
    SendLog(internal::LogLevel::INFO, format, args...);
#endif
  }
  void SetLogFormat(std::string format) { logFormat_ = format; }

 private:
  std::string name_;
  std::string logFormat_ = "[{time:08f}] [{component}] [{level}] {message}";
  inline static std::chrono::time_point<timer> start_time_ = timer::now();
  std::shared_ptr<Worker<const internal::Log>> worker_;

  template <typename... Args>
  void SendLog(internal::LogLevel const level, std::string const& format,
               Args&&... args) {
    auto time =
        std::chrono::duration<float>(timer::now() - start_time_).count();
    // TODO: figure out way to not use fmt::runtime
    worker_->AddJob({fmt::format(fmt::runtime(format), args...), level, name_,
                     logFormat_, time});
  }
};

namespace internal {

class LoggerRegistry {
 public:
  LoggerRegistry() {
    auto cout = std::shared_ptr<std::ostream>(&std::cout, [](std::ostream*) {});
    CreateSink("std::cout", cout);
  }

  // TODO: find out if count can crash in threaded context
  bool LoggerExists(std::string const& name) const {
    return loggers_.count(name) > 0;
  }

  void CreateLogger(std::string const& name, std::string const& sinkName = "") {
    auto sink = sinkName;
    if (!SinkExists(sinkName)) {
      sink = defaultSink_;
    }
    std::unique_lock loggerLock(loggerMutex_);
    std::shared_lock sinkLock(sinkMutex_);
    loggers_.insert({name, std::make_shared<Logger>(name, sinks_.at(sink))});
  }

  std::shared_ptr<Logger> GetLogger(std::string const& name) const {
    std::shared_lock loggerLock(loggerMutex_);
    return loggers_.at(name);
  }

  bool SinkExists(std::string const& name) const {
    return sinks_.contains(name);
  }

  // TODO: Add ability to have mulitple sinks for one worker
  void CreateSink(std::string const& name,
                  std::shared_ptr<std::ostream> const& stream) {
    auto newWorker =
        std::make_shared<Worker<const Log>>(CreateLoggerFunction(stream));
    newWorker->StartThread();

    std::unique_lock sinkLock(sinkMutex_);
    sinks_.insert({name, newWorker});
  }

  void SetDefaultSink(std::string name) {
    if (SinkExists(name)) {
      defaultSink_ = name;
    }
  }

 private:
  std::unordered_map<std::string, std::shared_ptr<Logger>> loggers_;
  std::unordered_map<std::string, std::shared_ptr<Worker<const Log>>> sinks_;
  std::string defaultSink_ = "std::cout";
  mutable std::shared_mutex loggerMutex_;
  mutable std::shared_mutex sinkMutex_;

  std::function<void(const Log&)> CreateLoggerFunction(
      std::shared_ptr<std::ostream> const& stream) const {
    return [=](Log const& log) {
      // TODO: figure out way to not use fmt::runtime
      *stream << fmt::format(fmt::runtime(log.format),
                             fmt::arg("time", log.time),
                             fmt::arg("component", log.component),
                             fmt::arg("level", LogLevelName[log.level]),
                             fmt::arg("message", log.message))
              << std::endl;
    };
  }
};

inline static LoggerRegistry loggerRegistry{};

}  // namespace internal

// TODO: Get and Set default logger that will be stored in static
// TODO: implement Error, Warn and Info that use the default logger

std::shared_ptr<Logger> GetLogger(std::string const& name = "Global",
                                  std::string const& filePath = "") {
  if (!internal::loggerRegistry.LoggerExists(name)) {
    if (filePath != "" && !internal::loggerRegistry.SinkExists(filePath)) {
      auto fileHandle = std::make_shared<std::ofstream>(filePath);
      internal::loggerRegistry.CreateSink(filePath, fileHandle);
    }
    internal::loggerRegistry.CreateLogger(name, filePath);
  }
  return internal::loggerRegistry.GetLogger(name);
}

void SetDefaultSink(std::string const& name) {
  internal::loggerRegistry.SetDefaultSink(name);
}

}  // namespace async_lib

#endif  // ASYNC_LIB_LOGGER_HPP
