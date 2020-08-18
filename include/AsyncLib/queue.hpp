#ifndef ASYNC_LIB_QUEUE_HPP
#define ASYNC_LIB_QUEUE_HPP

#include <atomic>

#ifndef NDEBUG
#include <assert.h>
#endif

namespace async_lib {

constexpr std::size_t DEFAULT_QUEUE_SIZE = 10;

// Allows us to differentiate between empty and full queue
constexpr std::size_t REAL_SIZE(std::size_t size) { return size + 1; }

template <class T, const std::size_t SIZE = DEFAULT_QUEUE_SIZE>
class Queue {
  typedef typename std::remove_const<T>::type StorageT;

 public:
  Queue() = default;
  ~Queue() = default;

  // TODO: implement these
  Queue(const Queue&) = delete;
  Queue& operator=(const Queue&) = delete;
  Queue(Queue&&) = delete;
  Queue& operator=(Queue&&) = delete;

  T& Front() const { return data_[front_]; }

  void Push(T&& data) {
    assert(Size() < SIZE);
    data_[back_++ % REAL_SIZE(SIZE)] = std::move(data);
  }

  void Push(const T& data) {
    assert(Size() < SIZE);
    data_[back_++ % REAL_SIZE(SIZE)] = data;
  }

  T Pop() {
    assert(Size() > 0);
    return data_[front_++ % REAL_SIZE(SIZE)];
  }

  std::size_t Size() const {
    return (back_ - front_ + REAL_SIZE(SIZE)) % REAL_SIZE(SIZE);
  }
  constexpr std::size_t Capacity() const { return SIZE; }

 private:
  mutable StorageT data_[REAL_SIZE(SIZE)];
  std::atomic_size_t front_{0};
  std::atomic_size_t back_{0};
};

}  // namespace async_lib

#endif  // ASYNC_LIB_QUEUE_HPP
