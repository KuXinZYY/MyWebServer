#ifndef ASYNLOGGING_H
#define ASYNLOGGING_H

#include"MutexLock.h"
#include"CountDownLatch.h"
#include"Thread.h"
#include<functional>
#include"LogFile.h"
#include"LogStream.h"
/*
多线程异步日志:
1.多个线程同时写一个日志文件,需要通过锁同步机制.
2.缓冲区写满一个,或者时间到,则通过条件变量通知后台进程写日志.
*/
class AsynLogging
{
public:
    AsynLogging(const std::string logName , int flushInterval = 3);
    ~AsynLogging()
    {
        if(running_)
            stop();
    }
    void start()
    {
        running_ = true;
        thread_.threadStart();
        latch_.wait();//主线程等待子线程运行threadFunc
    }
    void stop()
    {
        running_ = false;
        cond_.notify();//通知醒过来
        thread_.threadJoin();//回收thread的资源
    }
    void append(const char * logline , int len);//前端调用,最核心代码
private:
    AsynLogging(AsynLogging &rth);//禁止拷贝
    AsynLogging &operator=(AsynLogging &rth);//禁止赋值

    void threadFunc();//后端线程执行,最核心代码
    typedef FixedBuffer<kLargeBuffer> Buffer;//4M缓冲区类型定义
    typedef std::vector<std::shared_ptr<Buffer>> BufferVector;//通过vector管理多个缓冲区类型定义
    typedef std::shared_ptr<Buffer> BufferPtr;//缓冲区智能指针类型定义

    Thread thread_;

    BufferPtr currentBuffer_;//指向当前缓冲区
    BufferPtr backupBuffer_; //指向备用缓冲区
    std::string logName_;//日志名称
    const int flushInterval_;//每隔多少s,将当前缓冲区日志写入文件
    BufferVector buffers_;//用于管理多个缓冲区,可能出现一种情况,就是后端在写入日志的时候,前端写得过猛,所以可能需要多个缓冲区.

    MutexLock mutex_;
    Condition cond_;//用于通知后端线程的写日志.

    CountDownLatch latch_;//线程创建的时候,确保子线程立即运行.

    bool running_;//指示后端log线程是否运行.
};

#endif // ASYNLOGGING_H
