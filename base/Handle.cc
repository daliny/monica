#include "Handle.h"
#include "epoll.h"
#include "error.h"
#include <unistd.h>
#include <stdio.h>
#include <thread>

using namespace monica;

Handle::Handle(int fd, Epoll* epoll)
  : fd_(fd), ep(epoll) {  }

Handle::~Handle() {  }

void Handle::handle() {
  printf("The %lu thread get a connected fd: %d\n", 
                  std::this_thread::get_id(), fd_);
  for(;;) {
    int nread = 0;
    if((nread = read(fd_, buffer, MAXLINE)) == -1) {
      if(errno == EAGAIN) {
        ep->ctrl(EPOLL_CTL_MOD, fd_, EPOLLOUT|EPOLLET|EPOLLONESHOT);
        break;
      }
      linux_error("read error ", errno);
    }
    if(nread == 0) { 
      close(fd_);
      break;
    }
    else if(write(fd_, buffer, nread) == -1) {
      linux_error("write error ", errno);
    }
  }
}
