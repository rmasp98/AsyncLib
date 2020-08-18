
#include "AsyncLib/worker.hpp"

#include "gtest/gtest.h"
using namespace ::testing;

// tests
// Add capability to run multiple worker threads (will need safe pop when empty)

class AsyncWorkerTest : public Test {
 public:
  std::string out;
  std::function<void(std::string)> function = [&](std::string in) {
    out += in;
  };
  async_lib::Worker<std::string> worker{function};
};

TEST_F(AsyncWorkerTest, FlushDoesNothingIFQueueEmpty) {
  worker.Flush();
  ASSERT_EQ(out, "");
}

TEST_F(AsyncWorkerTest, CanAddJobToTheQueue) {
  worker.AddJob("Test");
  worker.Flush();
  ASSERT_EQ(out, "Test");
}

TEST_F(AsyncWorkerTest, CanAddMulitpleJobsAndFlushAll) {
  worker.AddJob("Test1");
  worker.AddJob("Test2");
  worker.Flush();
  ASSERT_EQ(out, "Test1Test2");
}

TEST_F(AsyncWorkerTest, CanStartWorkerThreadToProcessJobs) {
  worker.StartThread();
  worker.AddJob("Test3");
  // Needed to prevent kill before print
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  worker.KillThread();
  ASSERT_EQ(out, "Test3");
}

TEST_F(AsyncWorkerTest, StartingThreadTwiceKillsFirstThread) {
  worker.StartThread();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // This will terminate if first thread not joined
  worker.StartThread();
}

