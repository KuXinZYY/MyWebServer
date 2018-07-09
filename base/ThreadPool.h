#ifndef THREADPOLL_H
#define THREADPOLL_H
#include"MutexLock.h"
#include"Condition.h"
#include"Thread.h"
#include<deque>//任务队列
#include<vector>
#include<algorithm>
#include<binders.h>
#include<memory>
/*
基于生产者消费者模型写的线程池.
生产者:主线程
消费者:子线程
同步:一个互斥锁+两个条件变量
*/
using namespace std;

class ThreadPool{
public:
    typedef function<void ()> Task;//任务定义
    explicit ThreadPool(const string &poolName = string("ThreadPool"));
    ~ThreadPool();

    void initThreadPool(int numberThreads , int TaskQueueSize);//初始化线程池的大小
    void stopThreadPool();//停止线程池
    void addTask(const Task& task);

    const string& name() const
    {
        return poolName_;
    }
private:
    ThreadPool(ThreadPool &rth);//禁止拷贝
    ThreadPool &operator=(ThreadPool &rth);//禁止赋值

    /*
        线程池内部调用函数
    */
    void runInThread();//线程池内部每个线程最终执行函数，循环调用take获取任务，并执行任务，没有任务休眠，有任务执行。
    Task takeTask();//从任务队列中获取任务.

    MutexLock mutex_;//互斥锁
    Condition taskQueueNotFull_;//针对生产者
    Condition taskQueueNotEmpty_;//针对消费者
    string poolName_;//线程池名字
    size_t maxTaskQueueSize_;//任务队列大小
    size_t numberThreads_;
    deque<Task> taskQueue_;//任务队列
    vector< shared_ptr<Thread> > threads_;//存储线程对象指针,因为不允许拷贝.
    bool running_;
};






#endif // THREADPOLL_H
