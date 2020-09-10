#ifndef ASYNC_LIB_OBSERVER_HPP
#define ASYNC_LIB_OBSERVER_HPP

#include <assert.h>

#include <functional>
#include <list>
#include <memory>
#include <shared_mutex>

namespace async_lib {

template <typename... Args>
class Observer {
 public:
  explicit Observer(const std::function<void(Args...)> callback)
      : callback_(callback) {}

  // Ensures that Unsubscribe is called before deletion
  ~Observer() { assert(callback_ == nullptr); }

  // Should only be stored as shared_ptr
  Observer(const Observer&) = delete;
  Observer& operator=(const Observer&) = delete;
  Observer(Observer&&) = delete;
  Observer& operator=(Observer&&) = delete;

  void Callback(Args&... args) const {
    const std::unique_lock lock(mutex_);
    if (callback_) {
      callback_(args...);
    }
  }

  // Should be called before captured variables in callback_ go out of scope
  void Unsubscribe() {
    const std::unique_lock lock(mutex_);
    callback_ = nullptr;
  }

 private:
  std::function<void(Args...)> callback_;
  mutable std::mutex mutex_;
};

template <typename... Args>
class Subject {
 public:
  Subject() = default;
  ~Subject() = default;

  // Currently no reason to copy or move
  Subject(const Subject&) = delete;
  Subject& operator=(const Subject&) = delete;
  Subject(Subject&&) = delete;
  Subject& operator=(Subject&&) = delete;

  void Subscribe(std::weak_ptr<Observer<Args...>> observer) {
    std::unique_lock lock(mutex_);
    observers_.push_back(observer);
  }

  std::shared_ptr<Observer<Args...>> Subscribe(
      const std::function<void(Args...)> callback) {
    auto observer = std::make_shared<Observer<Args...>>(callback);
    Subscribe(observer);
    return observer;
  }

  void Notify(Args&... args) {
    std::unique_lock lock(mutex_);
    for (auto it = observers_.begin(); it != observers_.end();) {
      if (auto observer = it->lock()) {
        observer->Callback(args...);
        ++it;
      } else {
        it = observers_.erase(it);
      }
    }
  }

  void Notify(Args&&... args) { Notify(args...); }

  size_t Size() const { return observers_.size(); }

 private:
  std::list<std::weak_ptr<Observer<Args...>>> observers_;
  mutable std::mutex mutex_;
};

}  // namespace async_lib

#endif  // ASYNC_LIB_OBSERVER_HPP
