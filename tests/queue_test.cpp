#include "AsyncLib/queue.hpp"

#include "catch2/catch_test_macros.hpp"
#include "helpers.hpp"

TEST_CASE("Queue tests") {
  async_lib::Queue<int> queue;

  SECTION("Returns Zero Size For Empty Queue") { REQUIRE(queue.Size() == 0); }

  SECTION("Can Add Object To Queue") {
    queue.Push(5);
    REQUIRE(queue.Size() == 1);
  }

  SECTION("Can Retrieve Object From The Queue") {
    queue.Push(5);
    REQUIRE(queue.Front() == 5);
  }

  SECTION("Can Remove Object From The Queue") {
    queue.Push(5);
    queue.Pop();
    REQUIRE(queue.Size() == 0);
  }

  SECTION("Can Retrieve And Remove Object From The Queue") {
    queue.Push(5);
    REQUIRE(queue.Pop() == 5);
    REQUIRE(queue.Size() == 0);
  }

  SECTION("Can Add Multiple Objects To Queue And Retrieve All") {
    queue.Push(5);
    queue.Push(10);
    REQUIRE(queue.Pop() == 5);
    REQUIRE(queue.Pop() == 10);
  }

  SECTION("Popping Reduces Size Of Queue") {
    queue.Push(5);
    queue.Push(10);
    queue.Pop();
    REQUIRE(queue.Size() == 1);
  }

  SECTION("Can Define Maxium Size Of Queue") {
    auto queue = async_lib::Queue<int, 20>();
    REQUIRE(queue.Capacity() == 20);
  }

  SECTION("Can Push And Pop Multiple Times") {
    for (int i = 0; i < 5; i++) {
      queue.Push(i);
      REQUIRE(queue.Pop() == i);
    }
  }

  SECTION("Can Push And Pop More Times Than Size Of Queue") {
    auto queue = async_lib::Queue<int, 2>();
    for (int i = 0; i < 5; i++) {
      queue.Push(i);
      REQUIRE(queue.Pop() == i);
    }
  }

  SECTION("Size Still Correct When Queue Full") {
    auto queue = async_lib::Queue<int, 2>();
    queue.Push(5);
    queue.Push(1);
    REQUIRE(queue.Size() == 2);
  }

  SECTION("Size Still Correct When Queue Wraps Around") {
    auto queue = async_lib::Queue<int, 2>();
    queue.Push(5);
    queue.Push(1);
    queue.Pop();
    queue.Push(1);
    REQUIRE(queue.Size() == 2);
  }

  class TestElement {
   public:
    TestElement() = default;
    TestElement(int i) : data(i){};
    int data;
  };

  SECTION("Can Process Non Trivially Constructed Classes") {
    auto queue = async_lib::Queue<TestElement>();
    queue.Push(TestElement(1));
    REQUIRE(queue.Pop().data == 1);
  }

  SECTION("Can Process Const Classes") {
    auto queue = async_lib::Queue<const int>();
    queue.Push(1);
    REQUIRE(queue.Pop() == 1);
  }

  SECTION("Queue concurrent tests") {
    constexpr int numThreads = 10000;
    constexpr int numLoops = 10;
    auto queue = async_lib::Queue<int, numLoops * numThreads + 1>();

    SECTION("Can Safely Push In Thread Program") {
      RunInParallel(numThreads, [&](int thread) {
        for (int j = 0; j < numLoops; ++j) {
          queue.Push(thread);
        }
      });
      REQUIRE(queue.Size() == numLoops * numThreads);
    }

    SECTION("Can Safely Dequeue In Threaded Program") {
      for (int i = 0; i < numThreads * numLoops; ++i) {
        queue.Push(i);
      }
      RunInParallel(numThreads, [&](int) {
        for (int i = 0; i < numLoops; ++i) {
          queue.Pop();
        }
      });
      REQUIRE(queue.Size() == 0);
    }
  }
}
