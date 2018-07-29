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



