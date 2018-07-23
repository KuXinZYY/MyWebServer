#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H
/*
前面将每个线程封装一个事件循环
现在将上面封装了事件循环的线程封装成线程池,供主线程控制.
1.主线程获取子线程loop的指针.
2.主线程通过轮训,将事件监听分配到子线程,直接通过指针操作,并没有利用memcached的管道操作.(这种直接操作的方法不需要同步吗?)
*/
#include<vector>
#include"EventLoopThread.h"
#include"base/Logging.h"
class EventLoopThreadPool
{
public:
    EventLoopThreadPool(EventLoop *baseLoop , int numThreads);
    ~EventLoopThreadPool();
    void start();
    void quit();
    EventLoop *getNextLoop();
private:
    EventLoop *baseLoop_;//主线程的事件循环
    bool started_;//是否启动
    int numThreads_;//线程数量
    int next_;//获取下一个线程循环

    std::vector<std::shared_ptr<EventLoopThread>> threads;//存储EventLoopThread,引用计数为1
    std::vector<EventLoop *> loops_;//存储EventLoopThread中的EventLoop

};

#endif // EVENTLOOPTHREADPOOL_H
