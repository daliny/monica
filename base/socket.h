#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

namespace monica {
  class Socket {
  public:
    typedef struct sockaddr Sa;
    typedef struct addrinfo Ai;

    Socket(const char* host=nullptr, const char* port=nullptr, Socket::Ai *hints = nullptr);
    ~Socket(); // 判断res是否为空，否则delete
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    void getaddrinfo();
    void freeaddrinfo();
    int  create_socket();
    const char *gai_strerror(); // 使用gai_errno
    int  socket(int domain, int type, int protocol);
    void  bind(const Sa *addr, socklen_t addrlen);
  private:
    int sockfd;
    const char* host_; // 主机IP地址
    const char* port_; // 主机端口号
    Ai *hints_; // socket创建相关信息
    Ai *res; // getaddrinfo使用
    int sock_errno; // Socket类的错误码
    int gai_errno; // getaddrinfo产生的错误码
  }; // class Socket

} // namespace monica

#endif // BASE_SOCKET_H
