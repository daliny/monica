#ifndef BASE_TCPCONNECTION_H
#define BASE_TCPCONNECTION_H
#include "socket.h"
#include "acceptor.h"
#include "epoll.h"
#include "BoundedBlockingQueue.h"
#include <memory>

namespace monica {
  class TcpConnection {
    public:
      TcpConnection(int poll_size, int backlog = 20);
      ~TcpConnection();
      TcpConnection(const TcpConnection&) = delete;
      TcpConnection& operator=(const TcpConnection&) = delete;
      void create(const char* host, const char* port);
      void eventloop();
      int  getfd();
      void addfd(int fd, struct epoll_event *event);
    private:
      int listenfd;
      int backlog_; // 监听队列大小
      int events_size;
      Epoll poll_;
      BoundedBlockingQueue<int> queue_;
      std::unique_ptr<Socket> sock;
      std::unique_ptr<Acceptor> acceptor;
  };// TcpConnection
} // namespace monica

#endif  // BASE_TCPCONNECTION_H
