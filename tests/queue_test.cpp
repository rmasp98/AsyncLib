#include "AsyncLib/queue.hpp"

#include <memory>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

using namespace ::testing;

class AsyncQueueTest : public Test {
 public:
  async_lib::Queue<int> queue;
};

TEST_F(AsyncQueueTest, ReturnsZeroSizeForEmptyQueue) {
  ASSERT_EQ(queue.Size(), 0);
}

TEST_F(AsyncQueueTest, CanAddObjectToQueue) {
  queue.Push(5);
  ASSERT_EQ(queue.Size(), 1);
}

TEST_F(AsyncQueueTest, CanRetrieveObjectFromTheQueue) {
  queue.Push(5);
  ASSERT_EQ(queue.Front(), 5);
}

TEST_F(AsyncQueueTest, CanRemoveObjectFromTheQueue) {
  queue.Push(5);
  queue.Pop();
  ASSERT_EQ(queue.Size(), 0);
}

TEST_F(AsyncQueueTest, CanRetrieveAndRemoveObjectFromTheQueue) {
  queue.Push(5);
  ASSERT_EQ(queue.Pop(), 5);
  ASSERT_EQ(queue.Size(), 0);
}

TEST_F(AsyncQueueTest, CanAddMultipleObjectsToQueueAndRetrieveAll) {
  queue.Push(5);
  queue.Push(10);
  ASSERT_EQ(queue.Pop(), 5);
  ASSERT_EQ(queue.Pop(), 10);
}

TEST_F(AsyncQueueTest, PoppingReducesSizeOfQueue) {
  queue.Push(5);
  queue.Push(10);
  queue.Pop();
  ASSERT_EQ(queue.Size(), 1);
}

TEST_F(AsyncQueueTest, CanDefineMaxiumSizeOfQueue) {
  auto queue = async_lib::Queue<int, 20>();
  ASSERT_EQ(queue.Capacity(), 20);
}

TEST_F(AsyncQueueTest, CanPushAndPopMultipleTimes) {
  for (int i = 0; i < 5; i++) {
    queue.Push(i);
    ASSERT_EQ(queue.Pop(), i);
  }
}

TEST_F(AsyncQueueTest, CanPushAndPopMoreTimesThanSizeOfQueue) {
  auto queue = async_lib::Queue<int, 2>();
  for (int i = 0; i < 5; i++) {
    queue.Push(i);
    ASSERT_EQ(queue.Pop(), i);
  }
}

TEST_F(AsyncQueueTest, SizeStillCorrectWhenQueueFull) {
  auto queue = async_lib::Queue<int, 2>();
  queue.Push(5);
  queue.Push(1);
  ASSERT_EQ(queue.Size(), 2);
}

TEST_F(AsyncQueueTest, SizeStillCorrectWhenQueueWrapsAround) {
  auto queue = async_lib::Queue<int, 2>();
  queue.Push(5);
  queue.Push(1);
  queue.Pop();
  queue.Push(1);
  ASSERT_EQ(queue.Size(), 2);
}

class TestElement {
 public:
  TestElement() = default;
  TestElement(int i) : data(i){};
  int data;
};

TEST_F(AsyncQueueTest, CanProcessNonTriviallyConstructedClasses) {
  auto queue = async_lib::Queue<TestElement>();
  queue.Push(TestElement(1));
  ASSERT_EQ(queue.Pop().data, 1);
}

TEST_F(AsyncQueueTest, CanProcessConstClasses) {
  auto queue = async_lib::Queue<const int>();
  queue.Push(1);
  ASSERT_EQ(queue.Pop(), 1);
}

///////////////////////////////////////////////////////////////////////////
// Concurrent tests

class AsyncQueueConcurrentTest : public Test {
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

TEST_F(AsyncQueueConcurrentTest, CanSafelyPushInThreadProgram) {
  constexpr int numThreads = 10000;
  constexpr int numLoops = 10;
  auto queue = async_lib::Queue<int, numLoops * numThreads + 1>();
  RunInParallel(numThreads, [&](int thread) {
    for (int j = 0; j < numLoops; ++j) {
      queue.Push(thread);
    }
  });
  ASSERT_EQ(queue.Size(), numLoops * numThreads);
}

TEST_F(AsyncQueueConcurrentTest, CanSafelyDequeueInThreadedProgram) {
  constexpr int num_threads = 10000;
  constexpr int num_loops = 10;
  auto queue = async_lib::Queue<int, num_loops * num_threads + 1>();
  for (int i = 0; i < num_threads * num_loops; ++i) {
    queue.Push(i);
  }
  RunInParallel(num_threads, [&](int) {
    for (int i = 0; i < num_loops; ++i) {
      queue.Pop();
    }
  });
  ASSERT_EQ(queue.Size(), 0);
}
