#include "ThreadPool.h"
#include <string>
#include <cstdio>
#include <thread>
#include <memory>
#include <functional>

using namespace monica;

class Test {
public:
  Test(const std::string& str) : s(str){}
  void handle() {
    printf("%d:  handle -- thread's id %lu\n s = %s\n",++count, 
        std::this_thread::get_id(), s.c_str());
  }

private:
  std::string s;
  static int count;
};

int Test::count = 0;

int main() {
  ThreadPool pool(100);

  pool.start(4);
  for(int i = 0; i < 1000000; ++i) {
    char buf[30];
    sprintf(buf, "Task %3d", i);
    auto test = std::make_shared<Test>(Test(buf));
    pool.put_task(std::bind(&Test::handle, test));
  }

  pool.stop();
}
