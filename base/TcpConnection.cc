#include "TcpConnection.h"

#include <cstring>

#include <unistd.h>
#include <fcntl.h>

using namespace monica;

TcpConnection::TcpConnection(int poll_size, int backlog)
  : listenfd(-1),
    backlog_(backlog),
    events_size(poll_size),
    poll_(poll_size),
    queue_(poll_size), 
    sock(nullptr),
    acceptor(nullptr) {  }

TcpConnection::~TcpConnection() {  }

void TcpConnection::create(const char* host, const char* port) {
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

  listenfd = sfd;
}

void TcpConnection::eventloop() {
  struct epoll_event events[events_size];
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = listenfd;
  poll_.create();
  poll_.ctrl(EPOLL_CTL_ADD, listenfd, &event);

  while(true) {
    int nready = poll_.wait(events, -1);
    for(int i = 0; i < nready; ++i) {
      int fd = events[i].data.fd;
      if(fd == listenfd) {
        int connfd = acceptor->accept();
        event.events = EPOLLOUT|EPOLLIN|EPOLLHUP|EPOLLET;
        event.data.fd = connfd;
        fcntl(connfd, F_SETFL, O_NONBLOCK);
        poll_.ctrl(EPOLL_CTL_ADD, connfd, &event);
      } else {
        poll_.ctrl(EPOLL_CTL_DEL, fd, nullptr); // 避免数据竞争
        queue_.push(fd);
      }
    }
  }
}

int TcpConnection::getfd() {
  int fd = -1;
  queue_.wait_and_pop(fd);
  return fd;
}
void TcpConnection::addfd(int fd, struct epoll_event *event) {
  poll_.ctrl(EPOLL_CTL_ADD, fd, event);
}
