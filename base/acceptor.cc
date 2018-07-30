#include "acceptor.h"
#include "error.h"

#include <cstring>

#include <unistd.h>

using namespace monica;

Acceptor::Acceptor(int sockfd, int backlog) 
            : sockfd_(sockfd), 
              backlog_(backlog),
              acp_errno(0),
              addrlen(0) 
{  
  memset(&peer_addr, 0, sizeof(Acceptor::Sa));              
}
                            
Acceptor::~Acceptor() {
  if(sockfd_ >= 0) {
    ::close(sockfd_);
  }
}

void Acceptor::listen() {
  if(::listen(sockfd_, backlog_) == -1) {
    acp_errno = errno;
    linux_error("Acceptor::listen: ", acp_errno);
  }
}

int  Acceptor::accept() {
  int acceptfd = -1;
  if((acceptfd = ::accept(sockfd_, (Sa*)&peer_addr, &addrlen)) == -1) {
    acp_errno = errno;
    linux_error("Acceptor::accept: ", acp_errno);
  }
  return acceptfd;
}
