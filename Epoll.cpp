#include"Epoll.h"

Epoll::Epoll()
    : epollFd_(epoll_create(MAXFDS)),//初始化epoll
      events_(MAXFDS) //分配1000个元素的数组
{
    assert(epollFd_ > 0);
}
Epoll::~Epoll()
{

}
void Epoll::EpollAdd(FileEvent &fileEvent)//添加事件
{
    /*
        尽量简单,没有防御性编程特性
    */
    //Syscall param epoll_ctl(event) points to uninitialised byte(s)
    //避免valgrind的警告,很牛逼
    struct epoll_event event = {0};//avoid valgrind warning
    int fd = fileEvent.getFd();//获取当前请求的fd

    fileEvent.EqualAndUpdateLastEvents();//保存当前事件,为修改准备

    event.data.fd = fd;//返回标记哪个事件就绪
    event.events = fileEvent.getEvent() | EPOLLET ;//获取当前请求的事件
    if(epoll_ctl(epollFd_ , EPOLL_CTL_ADD , fd , &event) < 0){
        fileEvent.rest();
        perror("epoll_add error");
    }
}
void Epoll::EpollMod(FileEvent &fileEvent)//修改事件
{
    int fd = fileEvent.getFd();//获取当前请求的fd
    if(!fileEvent.EqualAndUpdateLastEvents()){//如果这次感兴趣事件,和上一次不一样,那么就是修改,如果一样了,就不需要修改可以少一个系统调用
        struct epoll_event event;
        event.data.fd = fd;//返回标记哪个事件就绪
        event.events = fileEvent.getEvent();//获取当前请求的事件
        if(epoll_ctl(epollFd_ , EPOLL_CTL_MOD , fd , &event) < 0){
            fileEvent.rest();
            perror("epoll_mod error");
        }
    }
}
void Epoll::EpollDel(FileEvent &fileEvent)//删除事件
{
    int fd = fileEvent.getFd();//获取当前请求的fd
    struct epoll_event event;
    event.data.fd = fd;//返回标记哪个事件就绪
    event.events = fileEvent.getEvent();//获取当前请求的事件
    if(epoll_ctl(epollFd_ , EPOLL_CTL_DEL , fd , &event) < 0){
        perror("epoll_mod error");
    }
    fileEvent.rest();
}
std::vector<struct epoll_event> Epoll::EpollDispatch(int timeout)//事件分发,阻塞时间由定时器事件决定
{
    std::vector<struct epoll_event> res;
    int eventnums =  epoll_wait(epollFd_, &(*events_.begin()), events_.size(), timeout);
    for(int i = 0 ; i < eventnums ;i++){//返回所有就绪事件
        res.push_back(events_[i]);
    }
    return res;//返回就绪的文件描述符
}
int Epoll::GetEpollFd()//获取epoll的fd
{
    return epollFd_;
}
