#ifndef ASYNC_LIB_WORKER_HPP
#define ASYNC_LIB_WORKER_HPP

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "AsyncLib/queue.hpp"

namespace async_lib {

// TODO: implement way to update the function?
// TODO: Add abiity to run several threads (automaticcally scale on queue size)
template <class T>
class Worker {
 public:
  explicit Worker(std::function<void(T&)> function) : function_(function) {}

  ~Worker() {
    KillThread();
    Flush();
  }

  // TODO: implement these
  Worker(const Worker&) = delete;
  Worker& operator=(const Worker&) = delete;
  Worker(Worker&&) = delete;
  Worker& operator=(Worker&&) = delete;

  void AddJob(T job) {
    queue_.Push(job);
    queue_wait_cv_.notify_all();
  }

  void Flush() {
    while (queue_.Size() > 0) {
      auto job = queue_.Pop();
      function_(job);
    }
  }

  void StartThread() {
    KillThread();
    thread_active_ = true;
    thread_ = std::make_unique<std::thread>([&]() {
      while (thread_active_) {
        if (queue_.Size() == 0) {
          std::unique_lock<std::mutex> lock(queue_wait_mutex_);
          queue_wait_cv_.wait(lock);
        }
        Flush();
      }
    });
  }

  void KillThread() {
    thread_active_ = false;
    queue_wait_cv_.notify_all();
    if (thread_ && thread_->joinable()) {
      thread_->join();
    }
  }

 private:
  Queue<const T> queue_;
  std::function<void(T&)> function_;
  std::unique_ptr<std::thread> thread_;
  bool thread_active_;
  std::condition_variable queue_wait_cv_;
  std::mutex queue_wait_mutex_;
};

}  // namespace async_lib

#endif  // ASYNC_LIB_WORKER_HPP
