#ifndef BASE_SERVER_H
#define BASE_SERVER_H

#include "ThreadPool.h"
#include "epoll.h"
#include "acceptor.h"
#include "socket.h"
#include "Handle.h"

#include <unistd.h>
#include <fcntl.h>

#include <memory>
#include <cstdlib>
#include <cstring>

namespace monica {
  template<typename T=Handle>
  class Server {
  public:
    Server(const Server&)=delete;
    Server& operator=(const Server&)=delete;

    Server(int pool_queue_size=20, size_t poll_size=20, int backlog = 20)
      : backlog_(backlog),
        epoll_(poll_size),
        events_size(poll_size),
        pool_(pool_queue_size),
        sock(nullptr),
        acceptor(nullptr){  }

    ~Server() {  }

    void work(const char* host, const char* port, size_t pool_size) {
      int listenfd = tcpconn(host, port);

      pool_.start(pool_size);
  
      struct epoll_event events[events_size];
      epoll_.create();
      epoll_.ctrl(EPOLL_CTL_ADD, listenfd, EPOLLIN);

      while(true) {
        int nready = epoll_.wait(events, -1);
        for(int i = 0; i < nready; ++i) {
          int fd = events[i].data.fd;
          if(fd == listenfd) {
            int connfd = acceptor->accept();
            fcntl(connfd, F_SETFL, O_NONBLOCK);
            epoll_.ctrl(EPOLL_CTL_ADD, connfd, EPOLLOUT|EPOLLET|EPOLLONESHOT);
          } else {
            auto handle = std::make_shared<T>(fd, &epoll_);
            pool_.put_task(std::bind(&T::handle, handle));
          }
        }
      }
    }

  protected:
    int tcpconn(const char* host, const char* port) {
      struct addrinfo hints;
      memset(&hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // 接收任何地址，环回地址除外
      hints.ai_flags |= AI_NUMERICSERV; // 必须有端口号
      sock = std::unique_ptr<Socket>(new Socket(host, port, &hints));
      int sfd = sock->create_socket();

      acceptor = std::unique_ptr<Acceptor>(new Acceptor(sfd, backlog_));
      acceptor->listen();

      return sfd;
    }

  private:
    int backlog_;
    Epoll epoll_;
    size_t events_size;
    ThreadPool pool_;
    std::unique_ptr<Socket> sock;
    std::unique_ptr<Acceptor> acceptor;
  }; // Server
} // namespace monica

#endif // BASE_SERVER_H
