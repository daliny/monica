#ifndef BASE_BLOCKINGQUEUE_H
#define BASE_BLOCKINGQUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <cassert>

namespace monica {

template<typename T>
class BlockingQueue {
  public:
    BlockingQueue()
      : mutex_(),
        notEmpty_(),
        queue_()
    {  }


    void push(const T& value) { // 压入
      {
        std::lock_guard<std::mutex> lk(mutex_);
        queue_.push(value);
      }
      notEmpty_.notify_one();
    }

    void push(T&& value) { // 压入右值（move语义）
      {
        std::lock_guard<std::mutex> lk(mutex_);
        queue_.push(std::move(value));
      }
      notEmpty_.notify_one();
    }

    void wait_and_pop(T& value) { // 等待获取弹出的数据（move语义）
      std::unique_lock<std::mutex> lk(mutex_);
      notEmpty_.wait(lk, [this]{ return !queue_.empty(); });
      assert(!queue_.empty());
      value = std::move(queue_.front());
      queue_.pop();
    }

    std::shared_ptr<T> wait_and_pop() { // 等待获取弹出的数据，返回弹出数据的`shared_ptr`（move语义）
      std::unique_lock<std::mutex> lk(mutex_);
      notEmpty_.wait(lk, [this]{ return !queue_.empty(); });
      assert(!queue_.empty());
      std::shared_ptr<T> res(std::make_shared<T>(std::move(queue_.front())));
      queue_.pop();
      return res;
    }

    size_t size() const { // 队列大小
      std::lock_guard<std::mutex> lk(mutex_);
      return queue_.size();
    }

  private:
    BlockingQueue(const BlockingQueue&); // = delete
    BlockingQueue& operator=(const BlockingQueue&); // = delete
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::queue<T> queue_;
};

} // namespace monica
#endif // BASE_BLOCKINGQUEUE_H
