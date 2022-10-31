#include "AsyncLib/unordered_map.hpp"

#include "gmock/gmock.h"
using namespace ::testing;

class AsyncUnorderedMapTest : public Test {};

TEST_F(AsyncUnorderedMapTest, SizeZeroOnDefaultConstruction) {
  async_lib::UnorderedMap<int, int> map;
  ASSERT_EQ(0, map.Size());
}

TEST_F(AsyncUnorderedMapTest, ConstructWithElements) {
  async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}, {3, 3}};
  ASSERT_EQ(3, map.Size());
}

TEST_F(AsyncUnorderedMapTest, ConstructFromUnorderedMap) {
  std::unordered_map<int, int> map{{1, 1}, {2, 2}, {3, 3}};
  async_lib::UnorderedMap<int, int> map2(map);
  ASSERT_EQ(3, map2.Size());
}

TEST_F(AsyncUnorderedMapTest, AccessElementWithBracketOperator) {
  async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}, {3, 3}};
  ASSERT_TRUE(map[1] == 1 && map[2] == 2 && map[3] == 3);
}

TEST_F(AsyncUnorderedMapTest, AccessCreatesDefaultIfDoesNotExist) {
  async_lib::UnorderedMap<int, int> map;
  ASSERT_TRUE(map[1] == int{});
}

TEST_F(AsyncUnorderedMapTest, AtAccessCanAccessConstMap) {
  async_lib::UnorderedMap<int, int> const map{{1, 1}, {2, 2}, {3, 3}};
  ASSERT_TRUE(map.At(1) == 1 && map.At(2) == 2 && map.At(3) == 3);
}

TEST_F(AsyncUnorderedMapTest, UpdateElementUsingAccessor) {
  async_lib::UnorderedMap<int, int> map;
  map[1] = 1;
  ASSERT_TRUE(map[1] == 1);
}

TEST_F(AsyncUnorderedMapTest, InsertElement) {
  async_lib::UnorderedMap<int, int> map;
  map.Insert(std::pair<int, int>(1, 1));
  ASSERT_TRUE(map[1] == 1);
}

TEST_F(AsyncUnorderedMapTest, InsertRangeFromAnotherMap) {
  async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}};
  async_lib::UnorderedMap<int, int> map2;
  map2.Insert(map.begin(), map.end());
  ASSERT_TRUE(map[1] == 1 && map[2] == 2);
}

TEST_F(AsyncUnorderedMapTest, InsertFromInitializerList) {
  async_lib::UnorderedMap<int, int> map;
  map.Insert({{1, 1}, {2, 2}});
  ASSERT_TRUE(map[1] == 1 && map[2] == 2);
}

TEST_F(AsyncUnorderedMapTest, EmplaceElement) {
  async_lib::UnorderedMap<int, int> map;
  map.Emplace(1, 1);
  ASSERT_TRUE(map[1] == 1);
}

TEST_F(AsyncUnorderedMapTest, EraseFromKey) {
  async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}};
  map.Erase(1);
  ASSERT_FALSE(map.Contains(1));
}

TEST_F(AsyncUnorderedMapTest, EraseFromIterator) {
  async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}};
  map.Erase(map.Find(1));
  ASSERT_FALSE(map.Contains(1));
}

TEST_F(AsyncUnorderedMapTest, ClearMap) {
  async_lib::UnorderedMap<int, int> map{{1, 1}, {2, 2}};
  map.Clear();
  ASSERT_FALSE(map.Contains(1) || map.Contains(2));
}

class AsyncUnorderedMapConcurrentTest : public Test {
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

TEST_F(AsyncUnorderedMapConcurrentTest, CanInsertKeyValueElementsInParalell) {
  async_lib::UnorderedMap<int, int> map;
  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      map.Insert((thread * 500) + i, 0);
    }
  });
  ASSERT_EQ(100 * 500, map.Size());
}

TEST_F(AsyncUnorderedMapConcurrentTest, CanInsertPairElementsInParalell) {
  async_lib::UnorderedMap<int, int> map;
  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      map.Insert({(thread * 500) + i, 0});
    }
  });
  ASSERT_EQ(100 * 500, map.Size());
}

TEST_F(AsyncUnorderedMapConcurrentTest, CanInsertInitialiserListsInParalell) {
  async_lib::UnorderedMap<int, int> map;
  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      auto key = (thread * 1000) + (i * 2);
      map.Insert({{key, 0}, {key + 1, 0}});
    }
  });
  ASSERT_EQ(100 * 1000, map.Size());
}

TEST_F(AsyncUnorderedMapConcurrentTest, CanInsertFromIteratorsInParalell) {
  async_lib::UnorderedMap<int, int> map;
  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      auto key = (thread * 1000) + (i * 2);
      async_lib::UnorderedMap<int, int> map2{{key, 0}, {key + 1, 0}};
      map.Insert(map2.begin(), map2.end());
    }
  });
  ASSERT_EQ(100 * 1000, map.Size());
}

TEST_F(AsyncUnorderedMapConcurrentTest, CanEmplaceElementsInParalell) {
  async_lib::UnorderedMap<int, int> map;
  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      map.Emplace((thread * 500) + i, 0);
    }
  });
  ASSERT_EQ(100 * 500, map.Size());
}

TEST_F(AsyncUnorderedMapConcurrentTest, CanEraseElementsInParalell) {
  async_lib::UnorderedMap<int, int> map;
  for (uint32_t i = 0; i < 500 * 100; ++i) {
    map.Insert(i, 0);
  }

  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      map.Erase((thread * 500) + i);
    }
  });
  ASSERT_EQ(0, map.Size());
}

TEST_F(AsyncUnorderedMapConcurrentTest, CanClearInParalell) {
  async_lib::UnorderedMap<int, int> map;
  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      map.Insert((thread * 500) + i, 0);
      map.Clear();
    }
  });
  ASSERT_EQ(0, map.Size());
}

TEST_F(AsyncUnorderedMapConcurrentTest, CanAccessInParalellWhileEditing) {
  async_lib::UnorderedMap<int, int> map;
  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      int key = (thread * 500) + i;
      map.Insert(key, key);
      ASSERT_EQ(key, map[key]);
      map.Erase(key);
    }
  });
}

TEST_F(AsyncUnorderedMapConcurrentTest, CanAccessWithAtInParalellWhileEditing) {
  async_lib::UnorderedMap<int, int> map;
  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      int key = (thread * 500) + i;
      map.Insert(key, key);
      ASSERT_EQ(key, map.At(key));
      map.Erase(key);
    }
  });
}

TEST_F(AsyncUnorderedMapConcurrentTest, CanCheckContainsInParalell) {
  async_lib::UnorderedMap<int, int> map;
  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      int key = (thread * 500) + i;
      map.Insert(key, key);
      ASSERT_TRUE(map.Contains(key));
      map.Erase(key);
    }
  });
}

TEST_F(AsyncUnorderedMapConcurrentTest, CanFindInParalell) {
  async_lib::UnorderedMap<int, int> map;
  RunInParallel(100, [&](int thread) {
    for (uint32_t i = 0; i < 500; ++i) {
      int key = (thread * 500) + i;
      map.Insert(key, key);
      map.Erase(map.Find(key));
    }
  });
}
