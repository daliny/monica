#include "epoll.h"
#include "error.h"

using namespace monica;

Epoll::Epoll(int events_size) 
  : epfd(-1),
    ep_errno(0),
    maxevents(events_size)
{  }

Epoll::~Epoll() {  }

void Epoll::create() {
  if((epfd = ::epoll_create1(EPOLL_CLOEXEC)) == -1) {
    ep_errno = errno;
    linux_error("Epoll::create: ", ep_errno);
  }
}

void Epoll::ctrl(int op, int fd, uint32_t events) {
  struct epoll_event event;
  event.events = events;
  event.data.fd = fd;
  if(::epoll_ctl(epfd, op, fd, &event) == -1) {
    ep_errno = errno;
    linux_error("Epoll::ctrl: ", ep_errno); // 需要改进，不同错误处理
  }
}

int Epoll::wait(Epoll::Ev* events, int timeout) {
  int nready = 0;
  if((nready = ::epoll_wait(epfd, events, maxevents, timeout)) == -1) {
    ep_errno = errno;
    linux_error("Epoll::wait: ", ep_errno);
  }
  return nready;
}
