#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop , int numThreads)
    : baseLoop_(baseLoop),
      started_(false),
      numThreads_(numThreads),
      next_(0)//首先获取第一个事件循环, 1 2 3 4 1 2 3 4 1 2 3 4通过轮询实现子线程的负载均衡.
{
    if(numThreads_ <= 0){
        LOG << "numThreads_ <= 0 ";
        abort();
    }
}

EventLoopThreadPool::~EventLoopThreadPool()
{
    //析构函数内部会退出EventLoopThread
}
void EventLoopThreadPool::start()
{
    started_ = true;
    for(int i = 0 ; i < numThreads_ ; i++){//创建子线程及事件循环
        char id[32];
        std::string WorkEventLoop = "WorkEventLoop";
        snprintf(id , sizeof(id) , "%d" , i+1);//将id转换成字符串

        std::shared_ptr<EventLoopThread> loopThread(new EventLoopThread(WorkEventLoop+id));//新建事件循环
        threads.push_back(loopThread);
        loops_.push_back(loopThread->startLoop());//事件循环运行起来,并将其Loop插入主线程管理
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    assert(started_ = true);
    int temp = (next_++)%numThreads_;//轮训
    return loops_[temp];//返回事件循环,给主线程控制
}
