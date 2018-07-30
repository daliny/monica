#include "socket.h"
#include "acceptor.h"
#include "error.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <unistd.h>

int main(int argc, char* argv[]) {
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

  monica::Acceptor acceptor_(sfd, 2);
  acceptor_.listen();

  int fd = acceptor_.accept();

  printf("Test Start\n");
  for(;;) {
    int nread = -1;
    if((nread = read(fd, buf, BUF_SIZE)) == -1) {
      monica::linux_error("read error: ", errno);
    }

    if(write(fd, buf, nread) == -1) {
      monica::linux_error("write error: ", errno);
    }

    if(strncmp(buf, "quit", 4) == 0) {
      printf("Test Success!!\n");
      break;
    }
  }
}
