#ifndef BASE_ACCEPTOR_H
#define BASE_ACCEPTOR_H
#include <sys/types.h>
#include <sys/socket.h>

namespace monica {
  class Acceptor {
    public:
      typedef struct sockaddr Sa;
      typedef struct sockaddr_storage Ss; // 最大的地址结构体
      Acceptor(int sockfd, int backlog);
      ~Acceptor();
      Acceptor(const Acceptor&) = delete;
      Acceptor& operator=(const Acceptor&) = delete;
      void listen();
      int  accept();
    private:
      int sockfd_;
      int backlog_; 
      int listenfd;
      int acp_errno;
      Ss  peer_addr; // 应该用最大的数据结构
      socklen_t addrlen;
  }; // class Acceptor
}// namespace monica

#endif //BASE_ACCEPTOR_H
