#ifndef ASYNC_LIB_POOL_HPP
#define ASYNC_LIB_POOL_HPP

#include <memory>
#include <mutex>
#include <unordered_set>
#include <vector>

#include "unordered_map.hpp"

using ElementId = uint32_t;

namespace async_lib {

class PoolBase {
 public:
  virtual uint64_t Size() const = 0;
  virtual uint64_t Capacity() const = 0;

  virtual void Remove(ElementId const) = 0;
};

template <typename ElementType>
class Pool : public PoolBase {
 public:
  explicit Pool(ElementId const initialSize = 0) { data_.reserve(initialSize); }

  uint64_t Size() const override {
    return data_.size() - freeList_.size() - garbageList_.size();
  }
  uint64_t Capacity() const override { return data_.capacity(); }

  bool Contains(ElementId const id) const {
    std::unique_lock garbageLock(garbageListMutex_);
    std::unique_lock freeLock(freeListMutex_);
    return !freeList_.contains(id) && !garbageList_.contains(id) &&
           id < data_.size();
  }

  std::weak_ptr<ElementType> Get(ElementId const id) {
    if (Contains(id)) {
      accessors_.Insert(
          {id, std::shared_ptr<ElementType>(
                   &data_[id], [&, id](ElementType*) { FreeGarbage(id); })});
      return accessors_.At(id);
    }
    return {};
  }

  ElementId Add(ElementType&& data) {
    std::unique_lock lock(freeListMutex_);
    if (freeList_.empty()) {
      std::unique_lock lock(dataMutex_);
      data_.push_back(std::forward<ElementType>(data));
      return data_.size() - 1;
    }

    auto freedIndex = *freeList_.begin();
    freeList_.erase(freedIndex);

    data_[freedIndex] = data;
    return freedIndex;
  }

  void Remove(ElementId const id) override {
    if (accessors_.Contains(id)) {
      {
        std::unique_lock lock(garbageListMutex_);
        garbageList_.insert(id);
      }
      accessors_.Erase(id);
    } else if (!garbageList_.contains(id)) {
      std::unique_lock lock(freeListMutex_);
      freeList_.insert(id);
    }
  }

 protected:
  void FreeGarbage(ElementId const id) {
    std::unique_lock garbageLock(garbageListMutex_);
    std::unique_lock freeLock(freeListMutex_);
    garbageList_.erase(id);
    freeList_.insert(id);
  }

 private:
  std::vector<ElementType> data_;
  std::mutex dataMutex_;

  std::unordered_set<ElementId> garbageList_;
  std::unordered_set<ElementId> freeList_;
  mutable std::mutex garbageListMutex_;
  mutable std::mutex freeListMutex_;

  UnorderedMap<ElementId, std::shared_ptr<ElementType>> accessors_;
};

}  // namespace async_lib

#endif  // ASYNC_LIB_POOL_HPP