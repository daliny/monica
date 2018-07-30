#include "TcpConnection.h"
#include "ThreadPool.h"
#include "error.h"
#include <stdio.h>
#include <thread>

#include <unistd.h>
#include <fcntl.h>

class Test {
  public:
    Test(int size) : tcp_(size, size) {  }

    void handle() {
      char buf[1024];
      int fd = tcp_.getfd();
      printf("The %lu thread get a connected fd: %d\n", 
                   std::this_thread::get_id(), fd);
      
      for(;;) {
        int nread = 0;
        if((nread = read(fd, buf, 1024)) == -1) {
          if(errno == EAGAIN) {
            continue;
          }
          monica::linux_error("read error ", errno);
        }
        if(nread == 0) { 
          close(fd);
          break;
        }
        else if(write(fd, buf, nread) == -1) {
          monica::linux_error("write error ", errno);
        }
      }
      /*
      int nread = 0;
      if((nread = read(fd, buf, 1024)) == -1) {
        monica::linux_error("read error ", errno);
      }

      if(nread == 0) { 
        close(fd);
        return;
      }
      else if(write(fd, buf, nread) == -1) {
        monica::linux_error("write error ", errno);
      }

      struct epoll_event event;
      event.events = EPOLLOUT|EPOLLIN|EPOLLHUP;
      event.data.fd = fd;
      tcp_.addfd(fd, &event);*/
    }

    void create(const char* host, const char* port) {
      tcp_.create(host, port);
    }

    void doit() {
      tcp_.eventloop();
    }
  private:
    monica::TcpConnection tcp_;
};

int main(int argc, char* argv[])
{
  const size_t BUF_SIZE = 1024;
  char buf[BUF_SIZE];

  if(argc != 2) {
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
 
  monica::ThreadPool pool_(10);
  pool_.start(4);

  Test test(10);
  test.create(nullptr, argv[1]);  
  for(int i = 0; i < 10; ++i) {
    pool_.put_task(std::bind(&Test::handle, &test));
  }

  test.doit();
  pool_.stop();
}
