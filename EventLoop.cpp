#include "EventLoop.h"

EventLoop::EventLoop()
    : poll_(),
      FileEventPtrVector(MAXFD),
      timeEventManager_(),
      quit_(false),
      wakeupFileEvent_(new FileEvent(this)),
      pendingCallBack(),
      mutex_(),
      wakeupFd_(createEventfd()),
      threadID_(CurrentThread::tid())

{
    wakeupFileEvent_->setFd(wakeupFd_);
    wakeupFileEvent_->setEvent(EPOLLIN);
    wakeupFileEvent_->setReadHandle(std::bind(&EventLoop::readEventfd , this) );
    AddFileEvent(wakeupFileEvent_);//将当前唤醒事件加入Epoll
    //LOG << FileEventPtrVector.size();
}

EventLoop::~EventLoop()
{

}

//创建eventfd,用于唤醒epoll
int EventLoop::createEventfd()
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG << "Failed in eventfd";
        abort();
    }
    LOG << CurrentThread::name() << "("<<CurrentThread::tid() << ") "
        << "create EventFd sucess = " << evtfd << " !";
    return evtfd;
}
void EventLoop::readEventfd()//可读,则内部必须有数据处理
{
    uint64_t one = 1;
    ssize_t n = readn(wakeupFd_, &one, sizeof one);

    std::vector<CallBack> func;
    {//加锁,直接将二者内容交换
        MutexLockGuard lock(mutex_);//加锁
        func.swap(pendingCallBack);
    }
    for(size_t i = 0 ; i < func.size() ; i++)
        func[i]();//执行回调函数

    //打印执行队列中多少条回调函数
    LOG <<  CurrentThread::name() << "("<<CurrentThread::tid() << ") " << "readEventfd reads " << n
        << " and DoPendingCallBack "<< func.size() <<" Times!";
}
void EventLoop::writeEventfd()
{
    uint64_t one = 1;
    ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
    LOG << CurrentThread::name() << "("<<CurrentThread::tid() << ") "
        << "writeEventfd writes " << n ;
}
void EventLoop::AddFileEvent(FileEventPtr fileEvent)
{
        int fd = fileEvent->getFd();
        assert(fd < MAXFD);
        FileEventPtrVector[fd] = fileEvent;//通过FileEventPtrVector管理FileEvent
        poll_.EpollAdd(*FileEventPtrVector[fd]);//添加到epoll
        LOG << CurrentThread::name() << "("<<CurrentThread::tid() << ") "
            << "Add FileEventFd , The fd is " << fd;
}
void EventLoop::DelFileEvent(FileEventPtr fileEvent)
{
    //assert(fd < MAXFD);
    poll_.EpollDel(*fileEvent);//添加到epoll
}

void EventLoop::AddTimeEvent(CallBack TimeHandle, long long milliseconds , bool persist)
{
    timeEventManager_.addTimeEvent(TimeHandle , milliseconds , persist);
}
void EventLoop::EventLoopProcess()
{
    while(!quit_){
        std::vector<struct epoll_event> res;
        long long timeout  = timeEventManager_.SearchNearestTime();
        res = poll_.EpollDispatch(timeout);
        for(size_t i = 0 ; i < res.size() ; i++){
            int fd = res[i].data.fd;
            FileEventPtrVector[fd]->setRevent(res[i].events);//存储epoll就绪事件
            FileEventPtrVector[fd]->handleEvent();//根据revent_处理对应的就绪事件
        }
        timeEventManager_.handleExpiredEvent();//处理时间事件
    }
    //退出的时候,执行的回调函数,可以定义
    LOG << CurrentThread::name() << "("<<CurrentThread::tid() << ")" << " quit!";
}

void EventLoop::queueInLoop(const CallBack &func)//这个函数由主线程调用,因为任务由主线程添加.
{
    {
        MutexLockGuard lock(mutex_);//加锁
        pendingCallBack.push_back(func);
    }//因为在加入队列的时候,可能当前线程在读取,所以必须先加锁暂时保护.
    //if(!isInLoopThread())//如果不在其他线程中将函数入队,则通过写Eventfd通知本线程有任务来了,醒来之后要执行
        writeEventfd();
}
//void EventLoop::doPendingCallBack()//被唤醒,则证明回调了
//{

//}


