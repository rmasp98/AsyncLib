
#include "AsyncLib/worker.hpp"

#include "catch2/catch_test_macros.hpp"

// tests
// Add capability to run multiple worker threads (will need safe pop when empty)

TEST_CASE("Worker tests") {
  std::string out;
  std::function<void(std::string)> function = [&](std::string in) {
    out += in;
  };
  async_lib::Worker<std::string> worker{function};

  SECTION("Flush Does Nothing If Queue Empty") {
    worker.Flush();
    REQUIRE(out == "");
  }

  SECTION("Can Add Job To The Queue") {
    worker.AddJob("Test");
    worker.Flush();
    REQUIRE(out == "Test");
  }

  SECTION("Can Add Mulitple Jobs And Flush All") {
    worker.AddJob("Test1");
    worker.AddJob("Test2");
    worker.Flush();
    REQUIRE(out == "Test1Test2");
  }

  SECTION("Can Start Worker Thread To Process Jobs") {
    worker.StartThread();
    worker.AddJob("Test3");
    // Needed to prevent kill before print
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    worker.KillThread();
    REQUIRE(out == "Test3");
  }

  SECTION("Starting Thread Twice Kills First Thread") {
    worker.StartThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // This will terminate if first thread not joined
    worker.StartThread();
  }
}