## Monica

#### 阻塞队列（BlockingQueue）

阻塞队列分两种：有界阻塞队列，无界阻塞队列。

**有界阻塞队列**是指队列在创建时就确定它的大小，队列最大只能增长到设定的边界。

​	当队列为空时，消费者线程（从队列读取数据的线程）被阻塞着，进入等待状态，直到有生成者线程（向队列写入数据的线程）插入数据到队列，并通知消费者进程【队列非空】；当队列已满时，生成者线程被阻塞着，进入等待状态，直到有消费者线程来取走数据，并通知生产者线程【队列非满】。

​	由此可见，要实现**有界阻塞**队列，需要两个**条件变量**，一个表示【队列非空】，一个表示【队列非满】，为了实现线程安全，每个线程要读取/写入数据时，都要先加锁：

```c++
template<typename T>
class BoundedBlockingQueue {
public:
    BoundedBlockingQueue(size_t size);
    void push(T&& value);
    void wait_and_pop(T& value);
    std::shared_ptr<T> wait_and_pop();
    bool empty();
    bool full();
    size_t size();
private:
    BoundedBlockingQueue(const BoundedBlockingQueue&); // = delete
    BoundedBlockingQueue& operator=(const BoundedBlockingQueue&); // = delete
    std::queue<T> queue_;
    size_t size_;                       // 队列大小
    mutable std::mutex mutex_;          // 互斥锁
    std::condition_variable notEmpty_;  // [队列非空]
    std::condition_variable notFull_;   // [队列非满]
};
```

```c++
void wait_and_pop(T& value);
std::shared_ptr<T> wait_and_pop();
```

为什么上面的两个成员函数要这么设计呢？为什么不让数据直接返回？

考虑一种情况，当内存负载很大，所剩内存不够再拷贝一个元素时，那么当我们的`wait_and_pop()`是返回一个元素数据，而这个数据又无法被拷贝存放，那么，它将永远丢失了。这是我们不想要的！！

这也是，STL设计者为什么将取数据操作`top()` 和 弹出操作`pop()`分开的原因，为了避免在弹出数据后，返回的数据又无法拷贝导致，数据丢失。而在多线程环境下，要保证队列能够安全的被读取，必须把`top()` 和 `pop()` 合为一个接口`wait_and_pop()`，又为了避免数据丢失，所以，我们应该传入一个对象的引用来保存返回数据或者是返回一个指向数据的指针。



**无界阻塞队列**是指不队列的大小不受限制的。

​	当队列为空时，消费者线程被阻塞着，进入等待状态。直到生成者线程向队列写入数据，并通知消费者进程【队列非空】，在等待中的消费者进程再次加锁并检查队列是否为空，非空则读取数据，否则解锁继续等待。

​	可见，无界阻塞队列只需要一个条件变量【队列非空】，为了线程安全，每次操作都要加锁：

```c++
template<typename T>
class BlockingQueue {
public:
    BlockingQueue();
    void push(T&& value);
    void wait_and_pop(T& value);
    std::shared_ptr<T> wait_and_pop();
    size_t size();
private:
    BlockingQueue(const BlockingQueue&); // = delete
    BlockingQueue& operator=(const BlockingQueue&); // = delete
	mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::queue<T> queue_;
};
```



#### 线程池（Thread Pool）

​	线程池由一个**有界阻塞队列**和一个**线程数组**组成，线程数组中的线程启动后等待队列中的数据，一个生产者线程向队列中加入数据。这里的“数据”其实是一个函数对象，生产者线程将要处理的资源和一个操作绑定在一起，称为一个”数据“，消费者线程领一个后，就用这个操作来处理这个资源，一切安排的明明白白~

```c++
class ThreadPool {
 14       public:                                  
 15       typedef std::function<void()> task_type;          
 16       ThreadPool(size_t sizeOfqueue);                                               
 17       ~ThreadPool();                  
 18       ThreadPool(const ThreadPool&) = delete;
 19       ThreadPool& operator=(const ThreadPool&) = delete;
 20       void start(size_t sizeOfpool);  // 线程开始运行       
 21       void stop();  // 线程停止运行                         
 22       void put_task(task_type&& task); // 装入数据到队列      
 23       // task_type get_task();          
 24     private:                                 
 25       void handle();     // 数据执行函数                    
 26       std::vector<std::thread> pool_; // 线程池         
 27       BoundedBlockingQueue<task_type> queue_; // 任务队列
};
```

​	线程池停止运行的方式是向任务队列中添加与消费者线程数相同的“空数据”，每个消费线程领到一个“空数据”就停止运行，也保证了已经装入的数据能被处理。

​	有一个要注意的就是每次开一个线程池后，要先让消费者线程运行起来，即先调用`ThreadPool::start()`，再让生产者线程装入数据`ThreadPool::put_task()`，这是因为每次装入数据，生产者都会发信号通知正在等待的消费者线程，如果生产者先运行，那么数据装入后就没通知到消费者线程来取，数据就“死”在队列了。



#### 套接字（Socket）

​	Socket类是对底层socket相关函数的包装，包括错误处理。

```c++
10   class Socket {                                                                     
 11   public:
 12     typedef struct sockaddr Sa;
 13     typedef struct addrinfo Ai;
 14  
 15     Socket(const char* host=nullptr, const char* port=nullptr, Socket::Ai *hints = nullptr
 16     ~Socket(); // 判断res是否为空，否则delete
 17     Socket(const Socket&) = delete;
 18     Socket& operator=(const Socket&) = delete;
 19     void getaddrinfo();    // 对`getaddrinfo()`的包装                  
 20     void freeaddrinfo();   // 对`freeaddrinfo()`的包装
 21     int  create_socket();  // 使用`getaddrinfo()`、`freeaddrinfo()`、`socket()`来创建协议无关的套接字
 22     const char *gai_strerror(); // 对gai_strerror()的包装，使用gai_errno
 23     int  socket(int domain, int type, int protocol); // 对`socket()`的包装
 24     void  bind(const Sa *addr, socklen_t addrlen); // 对`bind()`的包装
 25   private:
 26     int sockfd;
 27     const char* host_; // 主机IP地址
 28     const char* port_; // 主机端口号
 29     Ai *hints_; // socket创建相关信息
 30     Ai *res; // getaddrinfo使用
 31     int sock_errno; // Socket类的错误码
 32     int gai_errno; // getaddrinfo产生的错误码
 33   }; // class Socket

```

**一些想法**

​	其实，hints可以写成类的成员变量，然后再通过传入不同类型标识（如TCP，UDP等）来设置hints的值，这就需要写死hints，可能方便了，但是灵活度就下降了，也可以重载构造函数，一个传入`hints`的版本，还有一些传入类型标识的版本，这里我只实现了前者，后者以后有空再写。

​	此外，这里的`Socket::create_socket()`函数，仅供服务器端使用，里面调用了`bind()`，也可以再传入一个标识，判断是服务器端就使用`bind()`，是客户端就使用`connect()`。完美～



#### 监听与连接(Acceptor)

​	Acceptor类是对`listen()`、`accept()`的包装与错误处理。为什么要分开单独为类呢？因为不是每个套接字都需要这些操作。其实，也可以写到socket类里～

```c++
  1   class Acceptor {
  2     public:
  3       typedef struct sockaddr Sa;
  4       typedef struct sockaddr_storage Ss;
      	  Acceptor(int sockfd, int backlog);
  5       ~Acceptor();
  6       Acceptor(const Acceptor&) = delete;
  7       Acceptor& operator=(const Acceptor&) = delete;
  8       void listen(); // 对`listen()`的包装
  9       int  accept(); // 对`accept()`的包装
 10     private:
 11       int sockfd_;
 12       int backlog_;
 13       int listenfd;
 14       int acp_errno;
 15       Ss  peer_addr; // 应该用最大的数据结构
 16       socklen_t addrlen;
 17   }; // class Acceptor
```

​	为了保证代码的协议无关性，必须让peer_addr的空间足够大，能容下任何协议的地址结构，因此使用`struct sockaddr_storage`。



#### 轮询(epoll)

epoll 能知道有数据到了，如果把一个文件描述符给了某个线程，下一次数据来的时候，其他线程会通过epoll读到相同的文件描述符！！！

解决办法：

每次将文件描述符加入poll的队列前，先将该文件描述符移出epoll实例，然后：

1. 消费者线程去读取一个文件描述符后，这个文件描述符就只属于它了，如果文件描述符被设为非阻塞，则它可以选择循环来读取，这时，每个线程只能处理一个文件描述符，直到该连接断开。
2. 如果文件描述符被设为阻塞，则消费者线程可以读取一个，处理完这次后，将文件描述符再次加入epoll实例，然后，读取下一个文件描述符。（这种方法不好，多个线程向同个epoll实例提交文件描述符，可能导致多个线程读出相同的文件描述符，这就错误了。）