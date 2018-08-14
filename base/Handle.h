#ifndef BASE_HANDLE_H
#define BASE_HANDLE_H

#include "epoll.h"
#include <memory>

namespace monica {
  class Handle {
    public:
      enum {MAXLINE=5120};
      Handle(int fd, Epoll* epoll);
      ~Handle();
      Handle(const Handle&)=delete;
      Handle& operator=(const Handle&) = delete;
      virtual void handle();

    private:
      int fd_;
      Epoll* ep;
      char buffer[MAXLINE];
  }; // Handle
} // namespace monica

#endif // BASE_HANDLE_H
