
#include "AsyncLib/logger.hpp"

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"
#include "helpers.hpp"

TEST_CASE("Logger tests") {
  async_lib::internal::LoggerRegistry registry;

  SECTION("Registry Contains StdOut Sink") {
    REQUIRE(registry.SinkExists("std::cout"));
  }

  SECTION("Can Create A New Sink") {
    CHECK(!registry.SinkExists("std::ostringstream"));
    registry.CreateSink("std::ostringstream",
                        std::make_shared<std::ostringstream>());
    REQUIRE(registry.SinkExists("std::ostringstream"));
  }

  SECTION("Can Create New Logger") {
    CHECK(!registry.LoggerExists("test"));
    registry.CreateLogger("test");
    REQUIRE(registry.LoggerExists("test"));
  }

  auto ss = std::make_shared<std::ostringstream>();
  registry.CreateSink("ss", ss);
  registry.SetDefaultSink("ss");
  registry.CreateLogger("MainTest");
  auto mainLogger = registry.GetLogger("MainTest");

  SECTION("Can Submit A Log") {
    mainLogger->Error("{} {}", "Hello,", "world!");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    REQUIRE_THAT(
        ss->str(),
        Catch::Matchers::Matches(
            "\\[[0-9.]+\\] \\[MainTest\\] \\[Error\\] Hello, world!\n"));
  }

  SECTION("Default Sink Not Updated If it does not Exist") {
    registry.SetDefaultSink("DoesNotExist");
    registry.CreateLogger("test");
    auto logger = registry.GetLogger("test");
    logger->Warn("Hello");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    REQUIRE(ss->str() != "");
  }

  SECTION("Sink Cannot Be Overwritten") {
    registry.CreateSink(
        "ss", std::shared_ptr<std::ostream>(&std::cout, [](std::ostream*) {}));
    registry.CreateLogger("Test", "ss");
    auto logger = registry.GetLogger("Test");
    logger->Info("DoStuff");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    REQUIRE(ss->str() != "");
  }

  SECTION("Logger Cannot Be Overwritten") {
    registry.CreateLogger("MainTest", "std::cout");
    auto logger = registry.GetLogger("MainTest");
    logger->Error("DoStuff");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    REQUIRE(ss->str() != "");
  }

  SECTION("Can Set Output Format Of Logs") {
    mainLogger->SetLogFormat("{message}");
    mainLogger->Warn("{} = {}", "Something", 15.6);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    REQUIRE(ss->str() == "Something = 15.6\n");
  }

  SECTION("Sets Log Timestamp Properly") {
    mainLogger->SetLogFormat("{time}");
    mainLogger->Info("Test");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto time1 = stof(ss->str());
    ss->str("");
    mainLogger->Warn("Test2");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto time2 = stof(ss->str());

    REQUIRE(time2 > time1);
  }

  SECTION("Get Logger Creates Default Logger if Not Exist") {
    async_lib::GetLogger();
    REQUIRE(async_lib::internal::loggerRegistry.LoggerExists("Global"));
  }

  SECTION("Get Logger Returns Pointer To Same Logger") {
    auto logger1 = async_lib::GetLogger("SameTest");
    auto logger2 = async_lib::GetLogger("SameTest");
    REQUIRE(logger1 == logger2);
  }

  SECTION("Get Logger Creates New Sink If Does Not Exist") {
    async_lib::GetLogger("SinkTest", "LogFile.log");
    REQUIRE(async_lib::internal::loggerRegistry.SinkExists("LogFile.log"));
  }

  SECTION("Logger Concurrent Tests") {
    SECTION("Can Create Loggers In Paralell") {
      constexpr int numThreads = 100;
      constexpr int numLoops = 10;
      RunInParallel(numThreads, [&](int thread) {
        for (int j = 0; j < numLoops; ++j) {
          std::string name = fmt::format("{}_{}", thread, j);
          registry.CreateLogger(name);
        }
      });
      for (int i = 0; i < numThreads; ++i) {
        for (int j = 0; j < numLoops; ++j) {
          std::string name = fmt::format("{}_{}", i, j);
          REQUIRE(registry.LoggerExists(name));
        }
      }
    }

    SECTION("Can Create Sinks In Paralell") {
      constexpr int numThreads = 100;
      constexpr int numLoops = 10;
      RunInParallel(numThreads, [&](int thread) {
        for (int j = 0; j < numLoops; ++j) {
          std::string name = fmt::format("{}_{}", thread, j);
          registry.CreateSink(name, ss);
        }
      });
      for (int i = 0; i < numThreads; ++i) {
        for (int j = 0; j < numLoops; ++j) {
          std::string name = fmt::format("{}_{}", i, j);
          REQUIRE(registry.SinkExists(name));
        }
      }
    }
  }
}