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
    void push(const T& value);
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

这是