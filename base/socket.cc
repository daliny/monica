#include "socket.h"
#include "error.h"

#include <cstdio>
#include <cstdlib>

using namespace monica;

Socket::Socket(const char* host, const char* port, Socket::Ai *hints)
                : sockfd(-1),
                  host_(host),
                  port_(port),
                  hints_(hints),
                  res(nullptr), 
                  sock_errno(0), 
                  gai_errno(0) {  }

Socket::~Socket() {
  if(sockfd != -1) {
    ::close(sockfd);
  }
  printf("~Socket() be called!!\n");
  if(res != nullptr) {
    freeaddrinfo();
  }
}

int Socket::socket(int domain, int type, int protocol) {
  sockfd = ::socket(domain, type, protocol);
  sock_errno = errno;
  if(sockfd == -1) {
    // TODO
    // error handle
    linux_error("Socket::socket", sock_errno);
  }
  return sockfd;
}

void Socket::bind(const Sa *addr, socklen_t addrlen) {
  if(::bind(sockfd, addr, addrlen) == -1) {
    sock_errno = errno;
    // TODO
    // error handle
    linux_error("Socket::bind", sock_errno);
  }
}

void Socket::getaddrinfo() {
  gai_errno = ::getaddrinfo(host_, port_, hints_, &res);
  
  if(gai_errno != 0) {
    fprintf(stderr, "Socket::getaddrinfo error: %s\n", gai_strerror());
    exit(EXIT_FAILURE);
  }
}

void Socket::freeaddrinfo() {
  ::freeaddrinfo(res);
  res = nullptr;
}

const char* Socket::gai_strerror() {
  return ::gai_strerror(gai_errno);
}

int Socket::create_socket() {
  getaddrinfo();

  Ai *rp = res;
  for(; rp != nullptr; rp = rp->ai_next) {
    sockfd = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(sockfd == -1) {
      continue;
    }
    if(::bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      break;
    }
    ::close(sockfd);
  }

  freeaddrinfo();

  if(rp == NULL) {
    fprintf(stderr, "Socket::create_socket: Could not bind\n");
    exit(EXIT_FAILURE);
  }

  return sockfd;
}

