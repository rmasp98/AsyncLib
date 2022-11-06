
#include "AsyncLib/observer.hpp"

#include "catch2/catch_test_macros.hpp"
#include "helpers.hpp"

TEST_CASE("Observer tests") {
  std::ostringstream ss;
  auto main_observer =
      std::make_shared<async_lib::Observer<int>>([&](int num) { ss << num; });

  SECTION("Can subscribe") {
    async_lib::Subject<int> subject;
    subject.Subscribe(main_observer);
    REQUIRE(subject.Size() == 1);
  }

  async_lib::Subject<int> main_subject;
  main_subject.Subscribe(main_observer);

  SECTION("Can Run Observer Function With Notify") {
    main_subject.Notify(1);
    REQUIRE(ss.str() == "1");
  }

  SECTION("Can Have Multiple Subscribers") {
    auto observer2 =
        std::make_shared<async_lib::Observer<int>>([&](int num) { ss << num; });
    main_subject.Subscribe(observer2);
    main_subject.Notify(1);
    REQUIRE(ss.str() == "11");

    observer2->Unsubscribe();
  }

  SECTION("Removes Deleted Observers From List") {
    {
      auto observer2 = std::make_shared<async_lib::Observer<int>>(
          [&](int num) { ss << num; });
      main_subject.Subscribe(observer2);
      observer2->Unsubscribe();
    }
    main_subject.Notify(1);
    REQUIRE(main_subject.Size() == 1);
  }

  SECTION("Notify Does Not Run On Deleted Observers") {
    {
      auto observer2 = std::make_shared<async_lib::Observer<int>>(
          [&](int num) { ss << num; });
      main_subject.Subscribe(observer2);
      observer2->Unsubscribe();
    }
    main_subject.Notify(1);
    REQUIRE(ss.str() == "1");
  }

  SECTION("Subscribe Can Create And Return Observer") {
    auto observer2 = main_subject.Subscribe([&](int num) { ss << num; });
    int notifynum = 1;
    main_subject.Notify(notifynum);
    REQUIRE(ss.str() == "11");
    observer2->Unsubscribe();
  }

  SECTION("Observer concurrent tests") {
    SECTION("Subscribe And Notify Thread Safe") {
      constexpr int numThreads = 100;
      constexpr int numLoops = 10;
      RunInParallel(numThreads, [&](int) {
        for (int j = 0; j < numLoops; ++j) {
          main_subject.Subscribe(main_observer);
          main_subject.Notify(1);
        }
      });
      // If it gets here then it has seg faulted above
      REQUIRE(true);
    }
  }

  main_observer->Unsubscribe();
}
