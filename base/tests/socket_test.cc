#include "socket.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <sys/un.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  const size_t BUF_SIZE = 1024;
  char buf[BUF_SIZE];

  if(argc != 2) {
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  
  {// 测试Socket::bind() Socket::socket()
    printf("Test Socket::bind() & Socket::socket()\n");
    struct sockaddr_un my_addr, peer_addr;
    memset(&my_addr, 0, sizeof(struct sockaddr_un));
    my_addr.sun_family = AF_UNIX;
    strncpy(my_addr.sun_path, "/somepath", sizeof(my_addr.sun_path)-1);

    monica::Socket sock_;
    int fd = sock_.socket(AF_UNIX, SOCK_DGRAM, 0);
    int on=1;  
    if((setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)  
    {
      perror("setsockopt failed");  
      exit(EXIT_FAILURE);  
    }
    sock_.bind((struct sockaddr*)&my_addr, sizeof(struct sockaddr_un));
    printf("Test Success(Socket::bind() & Socket::socket())\n");
    unlink("/somepath");
  }


  { // 测试Socket::getaddrinfo()
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;

    monica::Socket sock_(nullptr, argv[1], &hints);
    sock_.getaddrinfo();
    //return 0;
  }

  // 测试Socket::create_socket()
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  monica::Socket socket_(nullptr, argv[1], &hints);
  int sfd = socket_.create_socket();
  
  for(;;) {
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
    ssize_t nread = recvfrom(sfd, buf, BUF_SIZE, 0, 
            (struct sockaddr *) &peer_addr, &peer_addr_len);
    if(nread == -1)
      continue;

    char host[NI_MAXHOST], port[NI_MAXSERV];
    int s = getnameinfo((struct sockaddr *)&peer_addr, 
                        peer_addr_len, host, NI_MAXHOST, 
                        port, NI_MAXSERV, NI_NUMERICSERV);
    if(s == 0)
      printf("Received %zd bytes from %s:%s\n", nread, host, port);
    else
      fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

    if(sendto(sfd, buf, nread, 0, (struct sockaddr *)&peer_addr,
              peer_addr_len) != nread)
      fprintf(stderr, "Error sending response\n");

    if(strncmp(buf, "quit", 4) == 0) {
      printf("Test Success!!\n");
      break;
      //exit(EXIT_SUCCESS);
    }
  }
  return 0;
}
