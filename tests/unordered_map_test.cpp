#include "AsyncLib/unordered_map.hpp"

#include "catch2/catch_test_macros.hpp"
#include "helpers.hpp"

TEST_CASE("Unordered map tests") {
  SECTION("Size Zero On Default Construction") {
    async_lib::UnorderedMap<int, int> map;
    REQUIRE(0 == map.Size());
  }

  SECTION("Construct With Elements") {
    async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    REQUIRE(3 == map.Size());
  }

  SECTION("Construct From Unordered Map") {
    std::unordered_map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    async_lib::UnorderedMap<int, int> map2(map);
    REQUIRE(3 == map2.Size());
  }

  SECTION("Access Element With Bracket Operator") {
    async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}, {3, 3}};
    REQUIRE(1 == map[1]);
    REQUIRE(2 == map[2]);
    REQUIRE(3 == map[3]);
  }

  SECTION("Access Creates Default If Does Not Exist") {
    async_lib::UnorderedMap<int, int> map;
    REQUIRE(map[1] == int{});
  }

  SECTION("At Access Can Access Const Map") {
    async_lib::UnorderedMap<int, int> const map{{1, 1}, {2, 2}, {3, 3}};
    REQUIRE(1 == map.At(1));
    REQUIRE(2 == map.At(2));
    REQUIRE(3 == map.At(3));
  }

  SECTION("Update Element Using Accessor") {
    async_lib::UnorderedMap<int, int> map;
    map[1] = 1;
    REQUIRE(1 == map[1]);
  }

  SECTION("Insert Element") {
    async_lib::UnorderedMap<int, int> map;
    map.Insert(std::pair<int, int>(1, 1));
    REQUIRE(1 == map[1]);
  }

  SECTION("Insert Range From Another Map") {
    async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}};
    async_lib::UnorderedMap<int, int> map2;
    map2.Insert(map.begin(), map.end());
    REQUIRE(1 == map[1]);
    REQUIRE(2 == map[2]);
  }

  SECTION("Insert From Initializer List") {
    async_lib::UnorderedMap<int, int> map;
    map.Insert({{1, 1}, {2, 2}});
    REQUIRE(1 == map[1]);
    REQUIRE(2 == map[2]);
  }

  SECTION("Emplace Element") {
    async_lib::UnorderedMap<int, int> map;
    map.Emplace(1, 1);
    REQUIRE(1 == map[1]);
  }

  SECTION("Erase From Key") {
    async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}};
    map.Erase(1);
    REQUIRE(!map.Contains(1));
  }

  SECTION("Erase From Iterator") {
    async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}};
    map.Erase(map.Find(1));
    REQUIRE(!map.Contains(1));
  }

  SECTION("Clear Map") {
    async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}};
    map.Clear();
    REQUIRE(!map.Contains(1));
    REQUIRE(!map.Contains(2));
  }

  SECTION("Unordered map concurrent tests") {
    SECTION("Can Insert Key Value Elements In Paralell") {
      async_lib::UnorderedMap<int, int> map;
      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          map.Insert((thread * 500) + i, 0);
        }
      });
      REQUIRE(100 * 500 == map.Size());
    }

    SECTION("Can Insert Pair Elements In Paralell") {
      async_lib::UnorderedMap<int, int> map;
      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          map.Insert({(thread * 500) + i, 0});
        }
      });
      REQUIRE(100 * 500 == map.Size());
    }

    SECTION("Can Insert Initialiser Lists In Paralell") {
      async_lib::UnorderedMap<int, int> map;
      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          auto key = (thread * 1000) + (i * 2);
          map.Insert({{key, 0}, {key + 1, 0}});
        }
      });
      REQUIRE(100 * 1000 == map.Size());
    }

    SECTION("Can Insert From Iterators In Paralell") {
      async_lib::UnorderedMap<int, int> map;
      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          auto key = (thread * 1000) + (i * 2);
          async_lib::UnorderedMap<int, int> map2{{key, 0}, {key + 1, 0}};
          map.Insert(map2.begin(), map2.end());
        }
      });
      REQUIRE(100 * 1000 == map.Size());
    }

    SECTION("Can Emplace Elements In Paralell") {
      async_lib::UnorderedMap<int, int> map;
      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          map.Emplace((thread * 500) + i, 0);
        }
      });
      REQUIRE(100 * 500 == map.Size());
    }

    SECTION("Can Erase Elements In Paralell") {
      async_lib::UnorderedMap<int, int> map;
      for (uint32_t i = 0; i < 500 * 100; ++i) {
        map.Insert(i, 0);
      }

      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          map.Erase((thread * 500) + i);
        }
      });
      REQUIRE(0 == map.Size());
    }

    SECTION("Can Clear In Paralell") {
      async_lib::UnorderedMap<int, int> map;
      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          map.Insert((thread * 500) + i, 0);
          map.Clear();
        }
      });
      REQUIRE(0 == map.Size());
    }

    SECTION("Can Access In Paralell While Editing") {
      async_lib::UnorderedMap<int, int> map;
      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          int key = (thread * 500) + i;
          map.Insert(key, key);
          // TODO: this might not work
          REQUIRE(key == map[key]);
          map.Erase(key);
        }
      });
    }

    SECTION("Can Access With At In Paralell While Editing") {
      async_lib::UnorderedMap<int, int> map;
      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          int key = (thread * 500) + i;
          map.Insert(key, key);
          // TODO: this might not work
          REQUIRE(key == map.At(key));
          map.Erase(key);
        }
      });
    }

    SECTION("Can Check Contains In Paralell") {
      async_lib::UnorderedMap<int, int> map;
      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          int key = (thread * 500) + i;
          map.Insert(key, key);
          // TODO: this might not work
          REQUIRE(map.Contains(key));
          map.Erase(key);
        }
      });
    }

    // TODO: Not finished
    SECTION("Can Find In Paralell") {
      async_lib::UnorderedMap<int, int> map;
      RunInParallel(100, [&](int thread) {
        for (uint32_t i = 0; i < 500; ++i) {
          int key = (thread * 500) + i;
          map.Insert(key, key);
          map.Erase(map.Find(key));
        }
      });
    }
  }
}
