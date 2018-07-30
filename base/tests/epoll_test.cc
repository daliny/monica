#include "epoll.h"
#include "socket.h"
#include "acceptor.h"
#include "error.h"

#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[]) 
{
  const size_t BUF_SIZE = 1024;
  char buf[BUF_SIZE];

  if(argc != 2) {
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  monica::Socket socket_(nullptr, argv[1], &hints);
  int sfd = socket_.create_socket();

  monica::Acceptor acceptor_(sfd, 10);
  acceptor_.listen();

  printf("Test Start\n");
  monica::Epoll epoll_(10);
  struct epoll_event events[10];
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = sfd;
  epoll_.create();
  epoll_.ctrl(EPOLL_CTL_ADD, sfd, &event);

  for(;;) {
    int nready = epoll_.wait(events, -1);
    for(int i = 0; i < nready; ++i) {
      int fd = events[i].data.fd;
      if(fd == sfd) {
        int afd = acceptor_.accept();
        event.events = EPOLLOUT|EPOLLIN|EPOLLHUP|EPOLLET;
        event.data.fd = afd;
        fcntl(afd, F_SETFL, O_NONBLOCK); // 注意要把afd设为非阻塞
        epoll_.ctrl(EPOLL_CTL_ADD, afd, &event);
      } else {
        int nread = -1;
        if((nread = read(fd, buf, BUF_SIZE)) == -1) {
          if(errno == EAGAIN) {
            continue;
          }
          monica::linux_error("read error: ", errno);
        }

        if(write(fd, buf, nread) == -1) {
          monica::linux_error("write error: ", errno);
        }

        if(strncmp(buf, "quit", 4) == 0) {
          printf("Test Success!!\n");
          close(fd);
          break;
        }

      }
    }
  }
}

