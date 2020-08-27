
#include <memory>

#define LOG_LEVEL 4
#include "AsyncLib/logger.hpp"
#include "gmock/gmock.h"
using namespace ::testing;

class AsyncLoggerTest : public Test {
 public:
  AsyncLoggerTest() {
    registry.CreateSink("ss", ss);
    registry.SetDefaultSink("ss");
    registry.CreateLogger("MainTest");
    mainLogger = registry.GetLogger("MainTest");
  }

  std::shared_ptr<std::ostringstream> ss =
      std::make_shared<std::ostringstream>();
  async_lib::internal::LoggerRegistry registry;
  std::shared_ptr<async_lib::Logger> mainLogger;
};

TEST_F(AsyncLoggerTest, RegistryContainsStdOutSink) {
  ASSERT_TRUE(registry.SinkExists("std::cout"));
}

TEST_F(AsyncLoggerTest, CanCreateANewSink) {
  ASSERT_FALSE(registry.SinkExists("std::ostringstream"));
  registry.CreateSink("std::ostringstream",
                      std::make_shared<std::ostringstream>());
  ASSERT_TRUE(registry.SinkExists("std::ostringstream"));
}

TEST_F(AsyncLoggerTest, CanCreateNewLogger) {
  ASSERT_FALSE(registry.LoggerExists("test"));
  registry.CreateLogger("test");
  ASSERT_TRUE(registry.LoggerExists("test"));
}

TEST_F(AsyncLoggerTest, CanSubmitALog) {
  mainLogger->Error("{} {}", "Hello,", "world!");
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  ASSERT_THAT(ss->str(),
              ContainsRegex(
                  "\\[[0-9.]+\\] \\[MainTest\\] \\[Error\\] Hello, world!\n"));
}

TEST_F(AsyncLoggerTest, DefaultSinkNotUpdatedIfNotExist) {
  registry.SetDefaultSink("DoesNotExist");
  registry.CreateLogger("test");
  auto logger = registry.GetLogger("test");
  logger->Warn("Hello");
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  ASSERT_NE(ss->str(), "");
}

TEST_F(AsyncLoggerTest, SinkCannotBeOverwritten) {
  registry.CreateSink(
      "ss", std::shared_ptr<std::ostream>(&std::cout, [](std::ostream*) {}));
  registry.CreateLogger("Test", "ss");
  auto logger = registry.GetLogger("Test");
  logger->Info("DoStuff");
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  ASSERT_NE(ss->str(), "");
}

TEST_F(AsyncLoggerTest, LoggerCannotBeOverwritten) {
  registry.CreateLogger("MainTest", "std::cout");
  auto logger = registry.GetLogger("MainTest");
  logger->Error("DoStuff");
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  ASSERT_NE(ss->str(), "");
}

TEST_F(AsyncLoggerTest, CanSetOutputFormatOfLogs) {
  mainLogger->SetLogFormat("{message}");
  mainLogger->Warn("{} = {}", "Something", 15.6);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  ASSERT_EQ(ss->str(), "Something = 15.6\n");
}

TEST_F(AsyncLoggerTest, SetsLogTimeStampProperly) {
  mainLogger->SetLogFormat("{time}");
  mainLogger->Info("Test");
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  auto time1 = stof(ss->str());
  ss->str("");
  mainLogger->Warn("Test2");
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  auto time2 = stof(ss->str());

  ASSERT_GT(time2, time1);
}

TEST_F(AsyncLoggerTest, GetLoggerCreatesDefaultLoggerifNotExist) {
  async_lib::GetLogger();
  ASSERT_TRUE(async_lib::internal::loggerRegistry.LoggerExists("Global"));
}

TEST_F(AsyncLoggerTest, GetLoggerReturnsPointerToSameLogger) {
  auto logger1 = async_lib::GetLogger("SameTest");
  auto logger2 = async_lib::GetLogger("SameTest");
  ASSERT_EQ(logger1, logger2);
}

TEST_F(AsyncLoggerTest, GetLoggerCreatesNewSinkIfDoesNotExist) {
  async_lib::GetLogger("SinkTest", "LogFile.log");
  ASSERT_TRUE(async_lib::internal::loggerRegistry.SinkExists("LogFile.log"));
}

class AsyncLoggerConcurrentTest : public Test {
 public:
  void RunInParallel(int nThreads, std::function<void(int)>&& function) {
    std::vector<std::shared_ptr<std::thread>> threads;
    for (int i = 0; i < nThreads; ++i) {
      threads.push_back(std::make_shared<std::thread>(function, i));
    }
    for (auto& thread : threads) {
      thread->join();
    }
  }
};

TEST_F(AsyncLoggerConcurrentTest, CanCreateLoggersInParalell) {
  constexpr int numThreads = 100;
  constexpr int numLoops = 10;
  async_lib::internal::LoggerRegistry registry;
  RunInParallel(numThreads, [&](int thread) {
    for (int j = 0; j < numLoops; ++j) {
      std::string name = fmt::format("{}_{}", thread, j);
      registry.CreateLogger(name);
    }
  });
  for (int i = 0; i < numThreads; ++i) {
    for (int j = 0; j < numLoops; ++j) {
      std::string name = fmt::format("{}_{}", i, j);
      ASSERT_TRUE(registry.LoggerExists(name));
    }
  }
}

TEST_F(AsyncLoggerConcurrentTest, CanCreateSinksInParalell) {
  constexpr int numThreads = 100;
  constexpr int numLoops = 10;
  auto ss = std::make_shared<std::ostringstream>();
  async_lib::internal::LoggerRegistry registry;
  RunInParallel(numThreads, [&](int thread) {
    for (int j = 0; j < numLoops; ++j) {
      std::string name = fmt::format("{}_{}", thread, j);
      registry.CreateSink(name, ss);
    }
  });
  for (int i = 0; i < numThreads; ++i) {
    for (int j = 0; j < numLoops; ++j) {
      std::string name = fmt::format("{}_{}", i, j);
      ASSERT_TRUE(registry.SinkExists(name));
    }
  }
}
