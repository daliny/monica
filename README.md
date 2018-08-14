# monica

* LINDA
* 2018/7/25

#### 内容

这是我学习多线程并发编程做的练习项目。一个采用`reactor`模式（one eventloop per thread+非阻塞IO+线程池）的简易网络库，现在已实现基本框架。比如，你想要一个`echo`服务器，只需如下操作即可：

```c++
#include "Server.h"

#include <stdio.h> 
using namespace monica;

int main(int argc, char* argv[])
{
    if(argc < 2) {
      fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    Server<> server;
    server.work(nullptr, argv[1], 4);
}  
```

如果你想使用`reactor`模式，但是想实现自己的服务器处理方式，只需把你的`Handle`类写好，然后作为模板参数传给`Server`类即可：

```c++
// 例如，你的处理模块是`httpHandle`类
#include "httpHandle.h" // 添加你的处理模块的头文件
#include "Server.h"

#include <stdio.h> 
using namespace monica;

int main(int argc, char* argv[])
{
    if(argc < 2) {
      fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    Server<httpHandle> server; // 这里提供你的处理模块即可
    server.work(nullptr, argv[1], 4);
}  
```

后面我会实现`httpHandle`模块，这样就可以直接使用了～～

#### 环境

**语言**：C++11

**平台**：Linux

**编译工具**：cmake

#### TODO

~~线程安全队列（有界阻塞队列、无界阻塞队列）~~

~~线程池（基于有界阻塞队列）~~

~~Socket类~~

~~Acceptor类~~

~~Epoll类~~

~~TcpConnection类~~

~~Handle类~~

~~Server类~~

httpHandle类