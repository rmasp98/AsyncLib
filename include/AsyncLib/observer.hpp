#ifndef ASYNC_LIB_OBSERVER_HPP
#define ASYNC_LIB_OBSERVER_HPP

#include <assert.h>

#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>

namespace async_lib {

class ObserverBase {
 public:
  virtual ~ObserverBase() = default;
  virtual void Unsubscribe() = 0;
};

template <typename... Args>
class Observer : public ObserverBase {
 public:
  explicit Observer(std::function<void(Args...)> const& callback)
      : callback_(callback) {}

  // Ensures that Unsubscribe is called before deletion
  ~Observer() { assert(callback_ == nullptr); }

  // Should only be stored as shared_ptr
  Observer(const Observer&) = delete;
  Observer& operator=(const Observer&) = delete;
  Observer(Observer&&) = delete;
  Observer& operator=(Observer&&) = delete;

  // TODO: do we want a pass by const& version of this function?
  void Callback(Args&... args) const {
    const std::shared_lock lock(mutex_);
    if (callback_) {
      callback_(args...);
    }
  }

  // Should be called before captured variables in callback_ go out of scope
  void Unsubscribe() override {
    const std::unique_lock lock(mutex_);
    callback_ = nullptr;
  }

 private:
  std::function<void(Args...)> callback_;
  mutable std::shared_mutex mutex_;
};

class SubjectBase {
 public:
  virtual ~SubjectBase() = default;
};

template <typename... Args>
class Subject : public SubjectBase {
 public:
  Subject() = default;
  ~Subject() = default;

  // Currently no reason to copy or move
  Subject(Subject const&) = delete;
  Subject& operator=(Subject const&) = delete;
  Subject(Subject&&) = delete;
  Subject& operator=(Subject&&) = delete;

  void Subscribe(std::weak_ptr<Observer<Args...>> const& observer) {
    std::unique_lock lock(mutex_);
    observers_.push_back(observer);
  }

  std::shared_ptr<Observer<Args...>> Subscribe(
      std::function<void(Args...)> const& callback) {
    auto observer = std::make_shared<Observer<Args...>>(callback);
    Subscribe(observer);
    return observer;
  }

  // Reference removed to account for empty parameter pack
  // TODO: figure out how to allow pass by reference and rvalue
  void Notify(Args... args) {
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

  size_t Size() const { return observers_.size(); }

 private:
  std::list<std::weak_ptr<Observer<Args...>>> observers_;
  mutable std::mutex mutex_;
};

}  // namespace async_lib

#endif  // ASYNC_LIB_OBSERVER_HPP
