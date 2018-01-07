在谈论一个API的行为时，经常会用到下面两组修饰词：

  - 阻塞（blocking）与非阻塞（non-blocking）
  - 同步（sync）与异步（async）

大多数情况下，阻塞意味着同步，非阻塞意味着异步。既然含义相同，为什么使用两组词汇来描述呢？答案是**角度不同**。

API用户关心的是**从API请求发出到执行完成，自己的线程是否被阻塞**。而API实现者在接收到请求后，需要作出决策：1. 在当前线程**立即**执行，造成阻塞；2. 记录请求，另起线程**稍后**执行。**立即**被称为**同步**，**稍后**被称为**异步**。同步、异步指实现方式，阻塞、非阻塞指API的语义形式。实现者往往同时提供两种API语义，底层复用一种实现方式。
```
// 同步方式提供两种语义
void BlockApi() {
    SyncOperation()
}

void NonBlockApi() {
    PushRequestToQueue()
}
void WorkerThread() {
    while (true) {
        request = FetchRequestFromQueue()
        SyncOperation()
    }
}
```

```
// 异步方式提供两种语义
void BlockApi() {
    future = AsyncOperation()
    future.Wait()
}

void NonBlockApi() {
    AsyncOperation()
}
```

API语义之所以出现从阻塞到非阻塞的演进，最主要的原因是I/O（磁盘I/O、网络I/O）速度与CPU速度之间的差距越来越大，引入非阻塞API既可以减少浪费在空等上的CPU时间，提高CPU利用率，又有利于增大I/O并发度，提高I/O设备的利用率。