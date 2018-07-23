#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include"base/Thread.h"
#include"EventLoop.h"
#include<functional>
//封装eventloop在某个线程中
class EventLoopThread
{
public:
    EventLoopThread(const std::string &threadName);
    ~EventLoopThread();
    EventLoop *startLoop();

private:
    void threadFunc();
    bool exiting_;
    Thread thread_;
    EventLoop *loop_;
    CountDownLatch latch_;

};

#endif // EVENTLOOPTHREAD_H
