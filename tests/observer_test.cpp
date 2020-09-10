
#include "AsyncLib/observer.hpp"

#include <memory>
#include <string>
#include <thread>

#include "gtest/gtest.h"
using namespace ::testing;

class ObserverTest : public Test {
 public:
  ObserverTest() { main_subject.Subscribe(main_observer); }
  ~ObserverTest() { main_observer->Unsubscribe(); }

  std::ostringstream ss;
  std::shared_ptr<async_lib::Observer<int>> main_observer =
      std::make_shared<async_lib::Observer<int>>([&](int num) { ss << num; });
  async_lib::Subject<int> main_subject;
};

TEST_F(ObserverTest, CanSubscribe) {
  async_lib::Subject<int> subject;
  subject.Subscribe(main_observer);
  ASSERT_EQ(subject.Size(), 1);
}

TEST_F(ObserverTest, CanRunObserverFunctionWithNotify) {
  main_subject.Notify(1);
  ASSERT_EQ(ss.str(), "1");
}

TEST_F(ObserverTest, CanHaveMultipleSubscribers) {
  auto observer2 =
      std::make_shared<async_lib::Observer<int>>([&](int num) { ss << num; });
  main_subject.Subscribe(observer2);
  main_subject.Notify(1);
  ASSERT_EQ(ss.str(), "11");

  observer2->Unsubscribe();
}

TEST_F(ObserverTest, RemovesDeletedObserversFromList) {
  {
    auto observer2 =
        std::make_shared<async_lib::Observer<int>>([&](int num) { ss << num; });
    main_subject.Subscribe(observer2);
    observer2->Unsubscribe();
  }
  main_subject.Notify(1);
  ASSERT_EQ(main_subject.Size(), 1);
}

TEST_F(ObserverTest, NotifyDoesNotRunOnDeletedObservers) {
  {
    auto observer2 =
        std::make_shared<async_lib::Observer<int>>([&](int num) { ss << num; });
    main_subject.Subscribe(observer2);
    observer2->Unsubscribe();
  }
  main_subject.Notify(1);
  ASSERT_EQ(ss.str(), "1");
}

TEST_F(ObserverTest, SubscribeCanCreateAndReturnObserver) {
  auto observer2 = main_subject.Subscribe([&](int num) { ss << num; });
  int notifynum = 1;
  main_subject.Notify(notifynum);
  ASSERT_EQ(ss.str(), "11");
  observer2->Unsubscribe();
}

class ObserverConcurrentTest : public Test {
 public:
  ~ObserverConcurrentTest() { main_observer->Unsubscribe(); }

  std::ostringstream ss;
  std::shared_ptr<async_lib::Observer<int>> main_observer =
      std::make_shared<async_lib::Observer<int>>([&](int num) { ss << num; });
  async_lib::Subject<int> main_subject;

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

TEST_F(ObserverConcurrentTest, SubscribeAndNotifyThreadSafe) {
  constexpr int numThreads = 100;
  constexpr int numLoops = 10;
  RunInParallel(numThreads, [&](int) {
    for (int j = 0; j < numLoops; ++j) {
      main_subject.Subscribe(main_observer);
      main_subject.Notify(1);
    }
  });
  // If it gets here then it has seg faulted above
  ASSERT_TRUE(true);
}
