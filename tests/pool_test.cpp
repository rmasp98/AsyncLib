#include "AsyncLib/pool.hpp"

#include <memory>

#include "catch2/catch_test_macros.hpp"
#include "helpers.hpp"

TEST_CASE("Pool Test") {
  async_lib::Pool<int> pool;

  SECTION("Initial size defines the reserve of the underlying vector") {
    auto reservedPool = std::make_unique<async_lib::Pool<int>>(10);
    REQUIRE(10 == reservedPool->Capacity());
  }

  SECTION("Can get the size of the underlying vector") {
    REQUIRE(0 == pool.Size());
  }

  SECTION("Can add an element to the pool") {
    pool.Add(1);
    REQUIRE(1 == pool.Size());
  }

  auto id = pool.Add(1);

  SECTION("Can remove an element from the pool") {
    pool.Remove(id);
    REQUIRE(0 == pool.Size());
  }

  SECTION("Added element will assign to freed Ids first") {
    pool.Remove(id);
    REQUIRE(id == pool.Add(1));
  }

  SECTION("Get returns nullptr for invalid id") {
    REQUIRE(nullptr == pool.Get(5).lock());
  }

  SECTION("Can get the contents of an element") {
    REQUIRE(1 == *pool.Get(id).lock());
  }

  SECTION("Mutliple gets will return same shared pointer") {
    auto shared1 = pool.Get(id).lock();
    auto shared2 = pool.Get(id).lock();
    REQUIRE(3 == shared2.use_count());
  }

  SECTION("Get returns nullptr if accessing freed element") {
    pool.Remove(id);
    REQUIRE(nullptr == pool.Get(id).lock());
  }

  SECTION("Remove element twice has no effect on vector") {
    pool.Remove(id);
    pool.Remove(id);
    REQUIRE(0 == pool.Size());
  }

  SECTION("Memory is not overwritten while shared pointer exists") {
    auto shared = pool.Get(id).lock();
    pool.Remove(id);
    REQUIRE(id != pool.Add(1));
  }

  SECTION("Cannot access after remove even if shared pointer exists") {
    auto shared = pool.Get(id).lock();
    pool.Remove(id);
    REQUIRE(nullptr == pool.Get(id).lock());
  }

  SECTION("Memory freed after remove and deletion of all shared pointers") {
    {
      auto shared = pool.Get(id).lock();
      pool.Remove(id);
    }
    REQUIRE(id == pool.Add(1));
  }

  SECTION("Size accounts for removed elements with shared ptrs") {
    auto shared = pool.Get(id).lock();
    pool.Remove(id);
    REQUIRE(0 == pool.Size());
  }

  SECTION("Remove called twice with shared ptr does not free memeory") {
    auto shared = pool.Get(id).lock();
    pool.Remove(id);
    pool.Remove(id);
    REQUIRE(id != pool.Add(1));
  }

  SECTION("Pool concurrent tests") {
    async_lib::Pool<int> conPool;

    SECTION("Can add, get and remove accessed elements in thread safe way") {
      constexpr int numThreads = 200;
      constexpr int numLoops = 500;
      RunInParallel(numThreads, [&](int) {
        for (int j = 0; j < numLoops; ++j) {
          auto id = conPool.Add(1);
          conPool.Get(id);
          conPool.Remove(id);
        }
      });
      REQUIRE(0 == conPool.Size());
    }
  }
}