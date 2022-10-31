#ifndef ASYNC_LIB_UNORDERED_MAP_HPP
#define ASYNC_LIB_UNORDERED_MAP_HPP

#include <cstdint>
#include <initializer_list>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>

namespace async_lib {

template <class Key, class Value, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>,
          class Allocator = std::allocator<std::pair<const Key, Value>>>
class UnorderedMap {
 public:
  using MapType = std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>;
  using ElementType = std::pair<Key const, Value>;

  UnorderedMap() = default;
  UnorderedMap(std::initializer_list<ElementType> list) : map_{list} {}
  explicit UnorderedMap(MapType const& map) : map_(map) {}

  template <typename InputIterator>
  UnorderedMap(InputIterator first, InputIterator last) : map_{first, last} {}

  UnorderedMap(const UnorderedMap&) = default;
  UnorderedMap(UnorderedMap&&) = default;
  UnorderedMap& operator=(const UnorderedMap&) = default;
  UnorderedMap& operator=(UnorderedMap&&) = default;

  uint32_t Size() const { return map_.size(); }

  Value& operator[](Key const& key) {
    std::shared_lock lock{mutex_};
    return map_[key];
  }

  Value const& At(Key const& key) const {
    std::shared_lock lock{mutex_};
    return map_.at(key);
  }

  auto begin() { return map_.begin(); }
  auto end() { return map_.end(); }
  auto const begin() const { return map_.begin(); }
  auto const end() const { return map_.end(); }

  auto Find(Key const& key) {
    std::shared_lock lock{mutex_};
    return map_.find(key);
  }

  auto const Find(Key const& key) const { return map_.find(key); }

  auto Insert(Key const& key, Value const& value) {
    return Insert({key, value});
  }

  auto Insert(ElementType const& element) {
    std::unique_lock lock{mutex_};
    return map_.insert(element);
  }

  auto Insert(std::initializer_list<ElementType> list) {
    std::unique_lock lock{mutex_};
    return map_.insert(list);
  }

  template <class InputIterator>
  void Insert(InputIterator first, InputIterator last) {
    std::unique_lock lock{mutex_};
    map_.insert(first, last);
  }

  template <class... Args>
  auto Emplace(Args&&... args) {
    std::unique_lock lock{mutex_};
    return map_.emplace(std::forward<Args>(args)...);
  }

  auto Erase(auto id) {
    std::unique_lock lock{mutex_};
    return map_.erase(id);
  }

  void Clear() {
    std::unique_lock lock{mutex_};
    map_.clear();
  }

  bool Contains(Key const& key) const {
    std::shared_lock lock{mutex_};
    return map_.contains(key);
  }

 private:
  MapType map_;
  mutable std::shared_mutex mutex_;
};

}  // namespace async_lib

#endif