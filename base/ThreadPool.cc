#include "ThreadPool.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>

using namespace monica;

ThreadPool::ThreadPool(size_t sizeOfqueue) 
                    : pool_(),
                      queue_(sizeOfqueue){  }

ThreadPool::~ThreadPool() {
  if(!queue_.empty()) {
    stop();
  }
}

void ThreadPool::start(size_t sizeOfpool) {
  for(size_t i = 0; i < sizeOfpool; ++i) {
    pool_.push_back(std::thread(std::bind(&ThreadPool::handle, this)));
  }
}

void ThreadPool::stop() {
  size_t size = pool_.size();
  for(size_t i = 0; i < size; ++i) {
    put_task(nullptr); // 插入size个空任务, 每个线程取得则退出
  }

  for(size_t i = 0; i < pool_.size(); ++i) {
    pool_[i].join();
  }

  //std::for_each(pool_.begin(), pool_.end(), std::bind(&std::thread::join, std::placeholders::_1));
}

void ThreadPool::put_task(task_type&& task) {
  queue_.push(task);
}

/*
ThreadPool::task_type ThreadPool::get_task() {
  task_type task;  
  wait_and_pop(task);
  return task;
}
*/
void ThreadPool::handle() {
  try {
    while(true) {
      task_type task;
      queue_.wait_and_pop(task);
      //printf("queue's size: %lu\n", queue_.size());
      if(task == nullptr) break;
      task();
    }
  }
  catch (const std::exception& ex) {
    fprintf(stderr, "exception caught in ThreadPool\n");
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...) {
    fprintf(stderr, "unknown expection caught in ThreadPool\n");
    throw; // rethrow
  }
}
