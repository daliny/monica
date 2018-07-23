#include "BlockingQueue.h"

#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <cstdio>
#include <sys/types.h>
#include <unistd.h>

class Test{
public:
  Test(size_t numOfthreads):mut(), queue_(){
    for(size_t i = 0; i < numOfthreads; ++i) {
      pool_.push_back(std::thread(
            std::bind(&Test::threadFunc, this)
            ));
    }
  }

  void run(int times) {
    std::thread::id tid = std::this_thread::get_id();
    for(int i = 0; i < times; ++i) {
      char buf[32];
      snprintf(buf, sizeof buf, "hello %d", i);
      queue_.push(buf);
      printf("tid = %d, push data = %s, size = %zd\n", tid, buf, queue_.size());
    }
  }

  void joinAll() {
    for(size_t i = 0; i < pool_.size(); ++i) {
      queue_.push("stop");
    }

    for(size_t i = 0; i < pool_.size(); ++i) {
      pool_[i].join();
    }
  }
private:
  void threadFunc() {
    while(true) {
      {
        //std::lock_guard<std::mutex> lk(mut);
        std::cout << "wait_add_pop api\n";
        std::cout << "thread's id: " << std::this_thread::get_id() << '\n';
        std::string s = "";
        std::cout << "queue's size: " << queue_.size() << '\n';
        queue_.wait_and_pop(s);
        std::cout << "queue's size: " << queue_.size() << '\n';
        std::cout << "queue pop: " << s << '\n';
        if(s == "stop")
          break;
      }
      {
        //std::lock_guard<std::mutex> lk(mut);
        std::cout << "shared_ptr wait_and_pop api" << '\n';
        std::cout << "thread's id: " << std::this_thread::get_id() << '\n';
        std::cout << "queue's size: " << queue_.size() << '\n';
        auto p = queue_.wait_and_pop();
        std::cout << "queue's size: " << queue_.size() << '\n';
        std::cout << "queue's size: " << queue_.size() << '\n';
        std::cout << "queue pop: " << *p << '\n';
        if(*p == "stop")
          break;
      }
    }
  }

  std::mutex mut;
  std::vector<std::thread> pool_;
  monica::BlockingQueue<std::string> queue_;
};

int main() {
  printf("pid = %d, tid = %d\n", ::getpid(), std::this_thread::get_id());
  Test t(4);
  t.run(100);
  t.joinAll();
}
