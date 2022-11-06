
#include <functional>
#include <memory>
#include <thread>
#include <vector>

inline void RunInParallel(int nThreads, std::function<void(int)>&& function) {
  std::vector<std::shared_ptr<std::thread>> threads;
  for (int i = 0; i < nThreads; ++i) {
    threads.push_back(std::make_shared<std::thread>(function, i));
  }
  for (auto& thread : threads) {
    thread->join();
  }
}