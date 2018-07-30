#ifndef BASE_THREADPOOL_H
#define BASE_THREADPOOL_H

#include "BoundedBlockingQueue.h"

#include <condition_variable>
#include <mutex>
#include <functional>
#include <thread>
#include <vector>

namespace monica {
  class ThreadPool {
    public:
      typedef std::function<void()> task_type;
      ThreadPool(size_t sizeOfqueue);
      ~ThreadPool();
      ThreadPool(const ThreadPool&) = delete;
      ThreadPool& operator=(const ThreadPool&) = delete;
      void start(size_t sizeOfpool);
      void stop();
      void put_task(task_type&& task);
      // task_type get_task();
    private:
      void handle();
      std::vector<std::thread> pool_; // 线程池
      BoundedBlockingQueue<task_type> queue_; // 任务队列
  };

} // namespace monica
#endif // BASE_THREADPOOL_H
