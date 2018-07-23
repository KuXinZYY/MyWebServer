#ifndef EVENTLOOP_H
#define EVENTLOOP_H
/*
封装socket事件
*/
#include"Epoll.h"
#include"TimeEvent.h"
#include"FileEvent.h"
#include<sys/eventfd.h>
#include"base/Logging.h"
#include"Epoll.h"
#include<unistd.h>
#include<vector>
#include"Util.h"

class EventLoop
{
public:
    typedef std::function<void()> CallBack;
    typedef std::shared_ptr<FileEvent> FileEventPtr;
    EventLoop();
    ~EventLoop();
    void AddFileEvent(FileEventPtr fileEvent);//添加IO事件
    void DelFileEvent(FileEventPtr fileEvent);//删除IO事件
    void AddTimeEvent(CallBack TimeHandle, long long milliseconds , bool persist = false);//添加定时器事件事件
    void EventLoopProcess();//处理事件循环
    void quit()
    {
        quit_ = true;
        writeEventfd();//唤醒你可以退出主循环了
    }
    void queueInLoop(const CallBack &func);
    void doPendingCallBack();
    bool isInLoopThread() const//往其他线程写
    {
        return threadID_ == CurrentThread::tid();
    }
private:

    int createEventfd();//利用EventFd代替管道,启到通知作用
    void readEventfd();
    void writeEventfd();

    static const int MAXFD = 4096;//可以设定好扩容机制
    Epoll poll_;//IO复用
    std::vector<FileEventPtr> FileEventPtrVector;//存放事件智能指针
    TimeEventManager timeEventManager_;//小根堆管理时间事件

    bool looping_;
    int wakeupFd_;//通过eventfd唤醒
    bool quit_;
    FileEventPtr wakeupFileEvent_;//为了避免epoll一直阻塞在epoll_wait中唤醒事件,则提供异步唤醒功能
    std::vector<CallBack> pendingCallBack;//唤醒待处理的回调,非常方便多线程添加事件
    MutexLock mutex_;//互斥保护队列操作.
    const pid_t threadID_;//缓存当前EventLoop在哪个线程IO中执行
    //bool callingpendingCallBack_;//当前是否正在执行挂起回调函数,防止在执行的时候,继续写唤醒.
};

#endif // EVENTLOOP_H
