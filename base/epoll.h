#ifndef BASE_EPOLL_H
#define BASE_EPOLL_H

#include <sys/epoll.h>

namespace monica {

  class Epoll {
    public:
      typedef struct epoll_event Ev;
      Epoll(int events_size);
      ~Epoll();
      Epoll(const Epoll&) = delete;
      Epoll& operator=(const Epoll&) = delete;
      void create();
      void ctrl(int op, int fd, uint32_t event);
      int wait(Ev* events, int timeout);
    private:
      int epfd;
      int ep_errno;
      int maxevents;
  }; // class Epoll
} // namespace monica
#endif // BASE_EPOLL_H
