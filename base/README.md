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
3. 使用`EPOLLONESHOT`，设置后文件描述符只能被通知一次，通知完就文件描述符被epoll实例内部取消了，不会再通知该文件描述符上的事件，直到使用`epoll_ctl`和`EPOLL_CTL_MOD`重新设置掩码。

可能的话，你应该避免使用epoll来跨线程负载均衡，避免跨线程共享epoll文件描述符，避免共享epoll注册的文件描述符。避免fork，如果你必须fork，请在`execve`调用前关闭所有epoll注册的文件描述符。

之前总是读了10个连接就无法继续读取连接了，原来是我把线程池的阻塞队列长度设为10，而只压进去了10个任务，所以10个任务被执行完了后，就没有任务可读了，所以应该将每个任务设为无限循环读取更多连接。



1. 阻塞IO+EPOLLET|EPOLLONESHOT
2. 非阻塞IO+EPOLLET+ 循环（每次加入poll的队列时，将描述符从epoll实例删除）
3. 非阻塞IO+EPOLLET|EPOLLONESHOT + 循环



​	我们将内核看做是总管，每个到来的连接看成是一个个客户，而每个工作进程是工人。在没有客人来的时候，工人们就回自己的房间睡觉，一旦有客人来了，总管就会去敲门，叫醒工人来为客户服务。**每次总管每次的叫醒方式都是从头一间房间开始，到最后一个房间为止。**那么总管是来几个客人就叫醒几个工人呢？还是一有客人来就把工人都叫醒，没有接到客人的就又回去碎觉。如果一有客人来就把所有工人叫醒，这种方式就造成了浪费，总管叫醒所有工人的时间和正在碎觉（等待）的工人数量是成正相关的，也就是说，碎觉的工人越多，客人的等待被服务的时间就更长了。这就是所谓的“惊群效应”。



#### epoll工作原理

​	我们都知道调用`epoll_create()`会创建一个表示epoll实例的文件描述符`epfd`，为什么要用文件描述符来表示呢？这是因为epoll实例也应该可以用于轮询，使用文件描述符表示就可以很方便的让一组epoll实例像文件描述符那样轮询了。文件描述符只是一个标记而已，其实epoll实例的内部实现实际上是一个内核数据结构体`struct eventpoll`，定义在`fs/eventpoll.c`第180行。这个数据结构体保存了所有`epoll`实例工作需要的内容。分配这个`struct eventpoll`并返回一个文件描述符的代码可以在`fs/eventpoll.c`的第1767行找到，下面是它的一个小片段：

```c++
/*
 * Create the internal data structure ("struct eventpoll").
 */
error = ep_alloc(&ep);
```

`ep_alloc()`简单的从内核堆中分配足够的空间给`struct eventpoll`，并初始化它。

在这之后，`epoll_create()`将尝试从当前进程中获取一个未使用的文件描述符：

```c
/*
* Creates all the items needed to setup an eventpoll file. That is,
* a file structure and a free file descriptor.
*/
fd = get_unused_fd_flags(O_RDWR | (flags & O_CLOEXEC));
```

如果`epoll_create()`能够要到一个文件描述符，它将尝试从系统获取一个匿名结点（`anonymous inode`）。注意，`epoll_create()`将之前分配的`struct eventpoll`的首地址指针保存到了这个文件的私有数据域中（`private_data field`）。所以，任何系统调用都可以通过实例的文件描述符来操作这个`epoll`实例。这个让重新获取`struct eventpoll`变得非常容易且高效：

```c
file = anon_inode_getfile("[eventpoll]", &eventpoll_fops, ep,
                          O_RDWR | (flags & O_CLOEXEC));
```

在这之后，`epoll_create`将文件描述符绑定到匿名结点上，并返回这个文件描述符给调用进程：

```c
fd_install(fd, file);
return fd;
```

那么，另一个问题**：`epoll`实例是如何”记住“所有注册到实例上的文件描述符的呢？**

​	对此，epoll使用了一个非常常见的内核数据结构——**红黑树**，来保存所有注册到该实例的文件描述符。而树的根为`struct eventpoll`的`rbr`成员变量，在`ep_alloc()`函数中被初始化。

​	对于每个`epoll`实例所监控的每个文件描述符，红黑树上将有对应的结点指向它，每个结点是一个`struct epitem`结构体。`struct epitem`上也保存了一些重要的数据结构，它们将`epoll实例`被用来监控该文件描述符的生命周期。

​	在我们使用`epoll_ctl`来添加一个文件描述符到一个epoll实例时，内核首先使用`ep_find()`尝试查找是否有与之匹配的`struct epitem`（是否已经存在），代码可以在`fs/eventpoll.c`的973行找到。

​	因为红黑树是一颗二叉搜索树，这意味着我们在存储一个新节点前，每个`struct epitem`中必须有一个可以比较的键。对于`epoll`，这个键是一个叫做`struct epoll_filefd`的结构体，它是`struct epitem`的一部分，它的定义也很简单，在`fs/eventpoll.c`的第106行：

```c
struct epoll_filefd {
	struct file *file; // pointer to the target file struct corresponding to the fd
	int fd; // target file descriptor number
} __packed;
```

完成这个比较的函数是`ep_cmp_ffd()`，定义在`fs/eventpoll.c`的第326行：

```c
/* Compare RB tree keys */
static inline int ep_cmp_ffd(struct epoll_filefd *p1,
                            struct epoll_filefd *p2)
{
	return (p1->file > p2->file ? +1:
		   (p1->file < p2->file ? -1 : p1->fd - p2->fd));
}
```

`ep_cmp_ffd()`先比较`struct file`的内存地址，存放在**高地址**的那个被认为是**较大的**。如果内存地址相同，则有多个文件描述符指向同一个`struct file`（例如，通过`dup()`），`ep_cmp_ffd()`将比较它们的文件描述符，文件描述符大的那个就被认为是较大的。

如果将要插入的文件描述符在epoll实例中已经存在，`ep_cmp_ffd()`将返回`0`，插入失败，`epoll_ctl()`返回错误，并设置`errno`为`EEXIT`。

如果没有找到，`epoll_ctl()`将调用`ep_insert()`来添加新的文件描述符到红黑树中，同时，设置一些要求的数据结构和与接收事件通知相关的回调函数。

#### 第二部分

`ep_insert()`是epoll实现中最重要的一个函数。理解它是如何工作的就差不多理解了epoll是如何从它监督的文件描述符接收新的事件。

```c
user_watches = atomic_long_read(&ep->user->epoll_watches);
if (unlikely(user_watches >= max_user_watches))
	return -ENOSPC;
```

上面这段代码，`epoll_insert()`先检查确保已经监督的文件的总数没有超出最大值，最大值可以在`/proc/sys/fs/epoll/max_user_watches`中查看。否则，`ep_insert()`马上返回并将`errno`设为`ENOSPC`。

接着，`ep_insert()`从内核`slab allocator`中分配内存给`epitem`。分配成功后，将初始化它：

```c
/* Item initialization follow here ... */
INIT_LIST_HEAD(&epi->rdllink);
INIT_LIST_HEAD(&epi->fllink);
INIT_LIST_HEAD(&epi->pwqlist);
epi->ep = ep;
ep_set_ffd(&epi->ffd, tfile, fd);
epi->event = *event;
epi->nwait = 0;
epi->next = EP_UNACTIVE_PTR;
```

接着，`ep_insert()`将尝试注册回调到文件描述符中。在讨论这个之前，我们需要先了解一些重要的数据结构：

`poll_table`是一个重要的数据结构，被VFS的`poll()`使用（这个`poll()`不是那个轮询的系统调用，而是文件操作自己使用的）。它的定义如下：

```c
typedef struct poll_table_struct {
	poll_queue_proc _qproc;
	unsigned long _key;
} poll_table;
```

`poll_queue_proc`是一个函数回调类型，如下：

```c
typedef void (*poll_queue_proc)(struct file *, wait_queue_head_t *, struct poll_table_struct *);
```

`_key`实际存储的是对该文件描述符感兴趣的**事件掩码**。在`epoll`实现中，`_key`被设为`~0`，这意味着`epoll`可以为任何事件类型接收回调。这也让用户空间的应用能够通过调用随时`epoll_ctl()`来修改事件掩码，接收所有来自**VFS**的事件和在epoll内部过滤它们变得更容易了。



为了能更容易的在`poll_queue_proc`使用原来的`struct epitem`，epoll使用了一个简单的数据结构`struct ep_pqueue`来包装一个`poll_table`和一个指向`struct epitem`的指针：

```c
/* Wrapper struct used by poll queueing */
struct ep_pqueue {
	poll_table pt;
    struct epitem *epi;
};
```

`ep_insert()`接着初始化`struct ep_pqueue`。下面的代码先设置`struct ep_pqueue`的`epi`成员为我们将要添加的文件相应的`struct epitem`，然后设置`struct eq_pqueue`的`_qproc`成员为`ep_ptable_queue_proc()`和`_key`为`~0`：

```c
/* Initialize the poll table using the queue callback */
epq.epi = epi;
init_poll_funcptr(&epq.pt, ep_ptable_queue_proc);
```

下一步，`ep_insert()`将调用`ep_item_poll(epi, &epq.pt);`，这个函数将调用与文件类型相一致的`poll()`函数。我们用**Linux TCP socket** 的`poll()`版本来看看它是如何工作的：



`tcp_poll()`是TCP socket 的`poll()`实现。它可以在`net/ipv4/tcp.c`中找到：

```c
unsigned int tcp_poll(struct file *file, struct socket *sock, poll_table *wait)
{
	unsigned int mask;
	struct sock *sk = sock->sk;
	const struct tcp_sock *tp = tcp_sk(sk);

	sock_rps_record_flow(sk);

	sock_poll_wait(file, sk_sleep(sk), wait);

	// code omitted
}
```

`tcp_poll()`调用了`sock_poll_wait()`，并使用`sk_sleep(sk)`作为它的第二个参数，`wait`是我们传入的`poll_table`。



那么，`sk_sleep()`是什么？它仅仅是通过一个特定的`struct sock`来获取到要访问该事件的**等待队列**：

```c
static inline wait_queue_head_t *sk_sleep(struct sock *sk)
{
	BUILD_BUG_ON(offsetof(struct socket_wq, wait) != 0);
	return &rcu_dereference_raw(sk->sk_wq)->wait;
}
```

接着，`sock_poll_wait()`将会对这个等待队列做些什么呢？它将做些简单的检查，然后调用`poll_wait()`，参数与`sock_poll_wait()`相同。而`poll_wait()`将调用我们指定的回调函数并将这个等待队列传给它（`include/linux/poll.h`）：

```c
static inline void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p)
{
	if (p && p->_qproc && wait_address)
		p->_qproc(filp, wait_address, p);
}
```

在epoll中，`_qproc`就是`ep_ptable_queue_proc()`，定义在`fs/eventpoll.c`：

```c
/*
* This is the callback that is used to add our wait queue to the
* target file wakeup lists.
*/
static void ep_ptable_queue_proc(struct file *file, wait_queue_head_t *whead,
			 poll_table *pt)
{
	struct epitem *epi = ep_item_from_epqueue(pt);
	struct eppoll_entry *pwq;

	if (epi->nwait >= 0 && (pwq = kmem_cache_alloc(pwq_cache, GFP_KERNEL))) {
		init_waitqueue_func_entry(&pwq->wait, ep_poll_callback);
		pwq->whead = whead;
		pwq->base = epi;
		add_wait_queue(whead, &pwq->wait);
		list_add_tail(&pwq->llink, &epi->pwqlist);
		epi->nwait++;
	} else {
		/* We have to signal that an error occurred */
		epi->nwait = -1;
	}
}
```

首先，`ep_ptable_queue_proc()`将尝试恢复我们正在处理的文件的`struct epitem`。因为，`epoll`使用了一个包装结构`struct ep_pqueue`，从`poll_table`指针恢复`struct epitem`只需要一些简单的指针运算。

在这之后，`ep_ptable_queue_proc()`简单的分配足够的空间给`struct eppoll_entry`。这个结构可以看成是监督文件的等待队列和这个文件相应的`struct epitem`之间的粘合剂。这个对于`epoll`保持追踪这个监督文件的等待队列来说很重要。否则，**`epoll`将无法从这个等待队列中注销**。`struct eppoll_entry`也包括了等待队列（`pwq->wait`），将它和唤醒函数`ep_poll_callback()`绑定在一起（这里不太清楚？？可以看一下[这个](https://lwn.net/Articles/661424/)），每当有进程从这个等待队列被唤醒就会调用它的唤醒函数。`pwq->wait`可以说是整个epoll实现中最重要的了，因为它会被用来：

1. **监督与被监督的文件相关的事件的到来；**
2. **唤醒与事件相关的进程。**

在这之后，`ep_ptable_queue_proc()`将`pwq->wait`绑定到目标文件的等待队列`whead`上。并且将`struct eppoll_entry`通过它的`llink`添加到`struct epitem`的链接表上（`epi->pwqlist`），然后，`epi->nwait`加1，它表示`epi->pwqlist`的长度。

好，这里就有一个问题。为什么`epoll`需要让每个文件的`struct epitem`都使用一个链接表`linked list`来存储`struct eppoll_entry`，而不是只存储一个`struct eppoll_entry`就好了？

答案就是，我也不知道:) 。我能说的就是，除非你尝试在`epoll`实例中创建一些奇怪的循环，否则，`epi->pwqlist`将只保存一个`eppoll_entry`并且`epi->nwait`更可能是`1` 。



不过，这并不影响我接下来要说的——Linux是怎么通知`epoll`实例哪些被监督的文件的事件发生了呢？

还记得我们前面几节说了什么吗？:smile: **`epoll`绑定一个`wait_queue_entry_t`(原文的`wait_queue_t`已经被弃用，看[这里](https://github.com/torvalds/linux/commit/ac6424b981bce1c4bc55675c6ce11bfe1bbfa64f))** 到目标文件的等待队列上（它是一个`wait_queue_head_t`）。尽管`wait_queue_entry_t`常被用作唤醒机制，它仅仅是一个存储了一个函数指针的结构体，**当Linux想唤醒绑定在`wait_queue_head_t`上的`wait_queue_entry_t`时，就会调用这个函数。在这个函数中，`epoll`就可以选择要怎么做来处理这个唤醒信号，并且它并不一定要唤醒任何进程！我们在后面会看到，很多时候`ep_poll_callback()`被调用时，并没有进程被唤醒。**

还有一件事是值得强调的，`poll()`的唤醒机制是完全依赖于具体实现的。对于TCP socket文件，等待队列是`struct sock`的一个成员`sk_wq`。这也解释了为什么我们需要`ep_ptable_queue_proc()`回调来将我们自身添加到那个等待队列中。因为，不同的文件版本将放置等待队列到完全不同的位置上，所以，除了使用回调，我们没有其他方法来正确定位`wait_queue_head_t`。



嗯？那么`struct sock`里的`sk_wq`什么时候会被唤醒呢？好的，事实证明，Linux socket系统也是像VFS那样采用`"OO"`设计的，`struct sock`在`net/core/sock.c`定义了下面的钩子`hook`：

```c
void sock_init_data(struct socket *sock, struct sock *sk)
{
	// code omitted...
	 sk->sk_data_ready  =   sock_def_readable;
	sk->sk_write_space =  sock_def_write_space;
    // code omitted...
}
```

在`sock_def_readable()`和`sock_def_write_space()`内部，`wake_up_interruptible_sync_poll()`在`(struct sock)->sk_wq`去执行唤醒回调函数时被调用。



那么什么时候`sk->sk_data_ready()`和`sk->sk_write_space()`将被调用？嗯...这个依赖于实现。例如TCP socket， `sk->sk_data_ready()`将会在TCP连接完成三次握手时或者一个缓冲区已经接受一个特定的TCPsocket时被调用。`sk->sk_write_space()`将会在一个缓冲区状态发生变化，即`full->available`发生在那个socket时被调用。记住这个行为，它将是后面的一个主题——边缘触发模式。



#### 第三部分

在这最后两部分，我将讲解`epoll`整体是怎么工作的，和`epoll`是如何从监控文件描述符那里接收新的事件通知的。在这一部分，我将讲解`epoll`是如何存储这些事件通知和用户进程是如何来取走它们的。



正如我们之前提到过的，`ep_insert()`函数将当前的`epoll`实例绑定到监控文件描述符的等待队列上并将`ep_poll_callback()`注册为唤醒队列的唤醒函数。以下是`ep_poll_callback()`的部分代码：

```c
// fs/eventpoll.c
static int ep_poll_callback(wait_queue_t *wait, unsigned mode, int sync, void *key)
{
	int pwake = 0;
	unsigned long flags;
	struct epitem *epi = ep_item_from_wait(wait);
	struct eventpoll *ep = epi->ep;
```

首先，`ep_poll_callback()`尝试通过`ep_item_from_wait()`借助`wait`将`struct epitem`提取出来，记住我们之前说`struct eppoll_entry`是一个”胶水“结构体，提取`struct epitem`只需要一些简单的指针运算：

```c
static inline struct epitem *ep_item_from_wait(wait_queue_t *p)
{
	return container_of(p, struct eppoll_entry, wait)->base;
}
```

回到`ep_poll_callback()`，接着是锁住`struct eventpoll`结构体：

```c
spin_lock_irqsave(&ep->lock, flags);
```

然后，函数检查已发生的事件是否是用户要求`epoll`监控的。记住，**`ep_insert()`函数注册poll回调并将事件掩码设为`~0U`。**这样做有两个原因：1. 用户可能频繁的调用`epoll_ctl()`来改变监控事件，重复的注册poll回调就不高效了；2. 并不是所有的文件系统都遵循事件掩码，因此使用它会变得不太可靠。

```c
if (key && !((unsigned long) key & epi->event.events))
	goto out_unlock;
```

接着，`ep_poll_callback()`检查`epoll`实例是否尝试将事件传给用户空间（通过`epoll_wait()`或`epoll_pwait()`）。如果是，`ep_poll_callback()`绑定当前的`struct epitem`到当前`struct eventpoll`的一个单链表`ovflist`的头部：

```c
if (unlikely(ep->ovflist != EP_UNACTIVE_PTR)) {
	if (epi->next == EP_UNACTIVE_PTR) {
		epi->next = ep->ovflist;
		ep->ovflist = epi;
		if (epi->ws) {
			__pm_stay_awake(ep->ws);
		}
	}
	goto out_unlock;
}
```

因为我们从`struct eventpoll`获得了自旋锁，所以这段代码是无竞争的，即使是在[SMP](https://en.wikipedia.org/wiki/Symmetric_multiprocessing)环境下。



然后，`ep_poll_callback()`检查这个`struct epitem`是否已经在准备队列（ready queue）上。这个可能发生在用户程序没有调用`epoll_wait()`。如果没有在准备队列上，`ep_poll_callback()`将添加它到准备队列，该队列是`struct eventpoll`上的`rdllist`成员。

```c
if (!ep_is_linked(&epi->rdllink)) {
	list_add_tail(&epi->rdllink, &ep->rdllist);
	ep_pm_stay_awake_rcu(epi);
}
```

接着，`ep_poll_callback()`唤醒在`wq`和`poll_wait`等待队列上等待的进程。在用户正在使用`epoll_wait()`轮询事件并且还未超时时，`wq`被`epoll`自身使用。`poll_wait`是被`epoll`实现的`poll()`操作使用的，因为`epoll`实例的文件描述符也是可以轮询的！！

```c
if (waitqueue_active(&ep->wq))
	wake_up_locked(&ep->wq);
	if (waitqueue_active(&ep->poll_wait))
		pwake++;
```

接着，`ep_poll_callback()`释放自旋锁，它被要求在唤醒`poll_wait`（poll()的等待队列）之前实施。注意，我们不能在持有自旋锁的时候，就唤醒`poll_wait`等待队列，因为它可能添加`epoll`自身的文件描述符到该`epoll`自己的监控文件集中，如果没有释放锁，就会发生死锁。

```c
out_unlock:
	spin_unlock_irqrestore(&ep->lock, flags);
	if (pwake)
		ep_poll_safewake(&ep->poll_wait);
return 1;
```



在`epoll`，存储已准备好的文件描述符列表的方式是很直接的，`struct eventpoll`的成员`rdllist`（原文写`rdllink`应该是笔误）是双端链表的头，而这个链表的结点就是一个个独立的已经有事件发生了的`struct epitem`。



最后，我来讲一下，当用户程序调用`epoll_wait()`时，`epoll`是如何传送文件描述符链表的。`epoll_wait()`函数很简单（`fs/eventpoll.c`）。它简单的做了写错误检查，从文件描述符的`private_data`获取`struct eventpoll`,并调用`ep_poll()`将事件复制到用户空间。下面我将主要讲解`ep_poll()`函数：

```c
// fs/eventpoll.c
static int ep_poll(struct eventpoll *ep, struct epoll_event __user *events, int maxevents, long timeout)
{
	int res = 0, eavail, timed_out = 0;
	unsigned long flags;
	long slack = 0;
	wait_queue_t wait;
	ktime_t expires, *to = NULL;

	if (timeout > 0) {
		struct timespec end_time = ep_set_mstimeout(timeout);
		slack = select_estimate_accuracy(&end_time);
		to = &expires;
		*to = timespec_to_ktime(end_time);
	} else if (timeout == 0) {
		timed_out = 1;
		spin_lock_irqsave(&ep->lock, flags);
		goto check_events;
	}
```

`ep_epoll()`对`epoll_wait()`是否阻塞采用了不同的方法。阻塞时（`timeout > 0`），函数基于`timeout`计算`end_time`。在非阻塞时（`timeout == 0`），函数直接跳到`check_events:`部分。



**阻塞版本**

```
fetch_events:
	spin_lock_irqsave(&ep->lock, flags);

	if (!ep_events_available(ep)) {

		init_waitqueue_entry(&wait, current);
		__add_wait_queue_exclusive(&ep->wq, &wait);

		for (;;) {
			set_current_state(TASK_INTERRUPTIBLE);
			if (ep_events_available(ep) || timed_out)
				break;
			if (signal_pending(current)) {
				res = -EINTR;
				break;
			}

			spin_unlock_irqrestore(&ep->lock, flags);
			if (!schedule_hrtimeout_range(to, slack, HRTIMER_MODE_ABS))
				timed_out = 1; /* resumed from sleep */

			spin_lock_irqsave(&ep->lock, flags);
		}
		__remove_wait_queue(&ep->wq, &wait);

		set_current_state(TASK_RUNNING);
	}
```

在`fetch_events:`之前，需要先获取`struct eventpoll`的自旋锁，否则当我们调用`ep_events_available(ep)`去检查可用的新事件时，就会有不好的情况发生。如果没有事件，该函数将当前进程添加到我们之前提到的等待队列`ep`中。然后，函数间当前任务设置为`TASK_INTERRUPTIBLE`，解锁，并告诉调度器去重新调度，而且，如果特定的时间已用完，或者接收到一个信号，将设置一个内核`timer`来重新调度当前进程。

之后，当进程被唤醒了（无论是时间用完了，信号或者新的事件到了），`ep_poll()`重新获取`struct eventpoll`的自旋锁，将该进程从`wq`等待队列中删除，设置任务状态为`TASK_RUNNING`，并检查是否有感兴趣的事件发生，这就是`check_events:`部分的内容了。



#### `check_events:`

首先，在持有锁时，`ep_poll()`检查是否有已准备好了的事件。然后，再释放锁。

```c++
check_events:
	eavail = ep_events_available(ep);
	spin_unlock_irqrestore(&ep->lock, flags);
```

如果函数没有获得任何事件并且时间未用完，这种情况会在该函数碰巧遇到一个过早的唤醒时发生，它将会重新回到`fetch_events:`并再次等待。否则，`ep_poll()`将返回：

```
if (!res && eavail && !(res = ep_send_events(ep, events, maxevents)) && !timed_out)
	goto fetch_events;
return res;
```

#### 非阻塞版本

非阻塞（`timeout == 0`）版本很直白。它直接跳到`check_events`，而不等待还未准备好的事件。



### 第四部分

这一部分，我将讲解`epoll`是如何从内核空间传送事件给用户空间的，和触发模式实现有什么不同之处。



**用户空间交互**

用户进程要使用事件，那么就需要内核必须能够提供捕获到的事件给用户进程。这个工作主要有`epoll_wait()`来完成。

`epoll_wait()`系统调用很直白，在一些基本的检查后，该函数从`epoll`实例的文件描述符提取`eventpoll`结构体的指针，然后调用这个函数：

```c
// fs/eventepoll.c
error = ep_poll(ep, events, maxevents, timeout);
```



`ep_poll()`先检查用户是否设置了`timeout`值。如果设置了，函数将初始化一个等待队列条目(`wait_queue_entry_t`)，并设置它的`timeout`。否则，`timeout = 0`，函数直接跳到`check_events:`去拷贝事件。如果用户设定了`timeout`但是还没有事件到来（通过调用`ep_events_available(ep)`检查），`ep_poll()`将添加该进程到`ep->wq`等待队列，我们知道当某个事件完成后，它会回调`ep_poll_callback()`将唤醒等待在`ep->wq`队列上的进程。



函数通过调用`schedule_hrtimeout_range()`进入睡眠。下面是正在睡眠的进程被唤醒的三种方式：

1. 进程超时了。
2. 进程接收到一个信号。
3. 有新事件到了。
4. 什么事都没发生，调度器决定唤醒进程。

对于1-3，函数设置相应的标识并退出等待循环。对于4，函数简单的重新进入睡眠。



之后，`ep_poll()`继续执行`check_events:`部分。

`check_events:`先确定确实有事件可用，通过调用下面这个函数将事件拷贝给用户空间：

```c
ep_send_events(ep, events, maxevents)
```

`ep_send_events()`调用了`ep_scan_ready_list()`，并用`ep_send_events_proc()`作为它的回调。这里，`ep_scan_ready_list()`遍历`ready list`，并为每个准备好的事件调用`ep_send_events_proc()`。我们将看到，回调机制对于代码重用和安全是很必要的。



`ep_send_events()`先将`eventpoll`里的`rdllist`移接到它的本地变量`txlist`，（调用了`list_splice_init()`这个函数其实是交换了两个链表的内容，然后初始化`rdllist`为空了）并将`ep->ovflist`设为`NULL`。

为什么`epoll`作者要使用`ovflist`呢？为了高效！正如我们看到的，在`rdllist`被移接到`txlist`后，`ep_scan_ready_list()`设置`ovflist`为`NULL`（**这个很重要！！因为`ovflist`默认情况下是`EP_UNACTIVE_PTR`，将它设为`NULL`后，如果有事件已经准备好并回调`ep_poll_callback()`时，会把事件直接添加到`ovflist`，而不会添加到`rdllist`**），**使得`ep_poll_callback()`在链接新的事件到`ep->rdllist`时，不会被转换到用户空间了**，避免造成错误，记住内存复制是一个昂贵的操作。通过使用`ovflist`，`ep_scan_ready_list()`在复制事件到用户空间时，就不需要持有`ep->lock`自旋锁。所以，整体的性能会更好。



然后，`ep_send_events_proc()`将重新合并`txlist`到`ep->rdllist`上，并调用文件描述符的相关`poll()`来确定事件确实被触发了。为什么`epoll`要再次检查事件呢？因为要确保使用者注册的事件仍然有效。思考一下，当用户进程正在向一个文件描述符写数据时，此时该文件描述符因`EPOLLOUT`被添加到`ready list`。在用户进程完成写操作后，该文件描述符可能不再可以被写任何东西。不知所云？？？弃之。



**我有个疑问（译者注）**：`list_splice(&txlist, &ep->rdllist);`是将`txlist`上的内容和`ep->rdllist`上的内容交换了，那么后来新增到`ep->rdllist`的事件不就没添加到`ep->rdllist`上了吗？

哈哈！我知道啦：）**因为Linux的`doubly linked list`设计为头结点的`prev`指针指向最后一个元素，最后一个元素的`next`指针指向头结点**，所以`list_splice(&txlist, &ep->rdllist);`其实是将`txlist`的内容合并到`ep->rdllist`上了。



**边缘触发(ET)&电平触发(LT)**

```c
// ep_send_events_proc(struct eventpoll *ep, struct list_head *head, void *priv) 
else if (!(epi->event.events & EPOLLET)) {
    list_add_tail(&epi->rdllink, &ep->rdllist);
}
```

如果是LT模式，`ep_send_events_proc()`简单将事件添加回`ep->rdllist`，下次相同的文件描述符有事件触发还会被检查一遍，如果是`ET`模式，

*******

* epoll实例的文件描述符有一个私有的`struct eventpoll`用来保存追踪那些关联到这个epoll实例的文件描述符。`struct eventpoll`有一个等待队列，用来保持追踪所有正在`epoll_wait`同个epoll实例的进程。`struct eventpoll`也有一个用来保存所有当前可读/写的文件描述符的列表。
* 当你使用`epoll_ctl`添加一个文件描述符到epoll实例，epoll添加



首先，调用`epoll_create()`创建一个epoll实例，它会在内部实现一颗红黑树，用来保存添加到该实例的文件描述符信息，返回一个文件描述符。

调用`epoll_ctl()`添加一个文件描述符时，会先查找红黑树，看是否已存在，如果未存在就创建新节点`epitem`，初始化节点`epitem`，会将epoll的回调函数`ep_ptable_queue_proc()`绑定进来，然后调用`ep_item_poll`，它会调用文件描述符自己指定的`poll()`函数，而这个`poll()`函数又会调用用户传入的`poll_table`结构体中指定的函数，也就是`ep_ptable_queue_proc()`。

* 它为什么要这么大费周折的回调呢？
* 这是因为使用文件描述符自己的`poll()`就可以获取到该文件描述符的进程等待队列，然后，再回调`ep_ptable_queue_proc()`函数将进程等待队列保存到`epitem`中。