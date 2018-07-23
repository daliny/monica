#ifndef BASE_BOUNDEDBLOCKINGQUEUE_H
#define BASE_BOUNDEDBLOCKINGQUEUE_H

#include <mutex>
#include <queue>
#include <condition_variable>
#include <cassert>

namespace monica {

template<typename T>
class BoundedBlockingQueue {
public:
  BoundedBlockingQueue(size_t size)
    :queue_(),
     size_(size),
     mutex_(), 
     notEmpty_(), 
     notFull_() {  }

  void push(const T& value) {
    {
      std::unique_lock<std::mutex> lk(mutex_);
      notFull_.wait(lk, [this]{ queue_.size() != size_ });
      assert(queue_.size() != size_);
      queue_.push(value);
    }
    notEmpty_.notify_one();
  }
  
  void push(T&& value) {
    {
      std::unique_lock<std::mutex> lk(mutex_);
      notFull_.wait(lk, [this]{ return queue_.size() != size_; });
      assert(queue_.size() != size_);
      queue_.push(std::move(value));
    }
    notEmpty_.notify_one();
  }

  void pop(T& value) {
    std::unique_lock<std::mutex> lk(mutex_);
    notEmpty_.wait(lk, [this]{ return !queue_.empty(); });
    assert(!queue_.empty());
    value = std::move(queue_.front());
    queue_.pop();
    notFull_.notify_one();
  }

  shared_ptr<T> pop() {
    std::unique_lock<std::mutex> lk(mutex_);
    notEmpty_.wait(lk, [this]{ return !queue_.empty(); });
    assert(!queue_.empty());
    shared_ptr<T> res = make_shared(std::move(queue_.front()));
    queue_.pop();
    notFull_.notify_one();

    return res;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lk(mutex_);
    return queue_.empty();
  }

  bool full() const {
    std::lock_guard<std::mutex> lk(mutex_);
    return queue_.size() == size_;
  }

  size_t size() const {
    std::lock_guard<std::mutex> lk(mutex_);
    return queue_.size();
  }

private:
  BoundedBlockingQueue(const BoundedBlockingQueue&); // = delete
  BoundedBlockingQueue& operator=(const BoundedBlockingQueue&); // = delete
  
  std::queue<T> queue_;
  size_t size_;
  mutable std::mutex mutex_;
  std::condition_variable notEmpty_;
  std::condition_variable notFull_;
};

} // namespace monica
#endif //BASE_BOUNDEDBLOCKINGQUEUE_H
