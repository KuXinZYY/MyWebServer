#include "EventLoopThread.h"
#include<string>
//多线程时刻记住,在子线程中都需要给主线程留接口,
//因为始终是由主线程控制子线程运行.
EventLoopThread::EventLoopThread(const std::string &threadName)
    : exiting_(false) ,
      thread_(std::bind(&EventLoopThread::threadFunc , this) , threadName),
      loop_(NULL),
      latch_(1)
{

}
EventLoopThread::~EventLoopThread()//这个都是在主线程调用
{
    exiting_ = true;
    if(loop_ != NULL){
        loop_->quit();//线程EvenLoop退出
        thread_.threadJoin();//主线程收尾子线程退出
    }
}

EventLoop *EventLoopThread::startLoop()//主线程调用
{
    thread_.threadStart();
    latch_.wait();//等待创建事件循环之后,才可以返回其地址
    return loop_;
}
void EventLoopThread::threadFunc()//线程中执行的函数,创建eventloop
{
    EventLoop loop;//对象生命周期同工作线程生命周期,因为栈上创建对象,所以不必关心内存泄露问题.
    loop_ = &loop;
    latch_.countdown();//通知事件循环已经创建成功
    loop_->EventLoopProcess();//事件循环搞起来
    loop_ = NULL;
}
