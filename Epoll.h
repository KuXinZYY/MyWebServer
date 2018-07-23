#ifndef EPOLL_H
#define EPOLL_H

#include<sys/epoll.h>
#include"FileEvent.h"
#include<assert.h>
#include<vector>
/*
封装事件IO复用
*/

class Epoll
{
public:
    Epoll();
    ~Epoll();
    void EpollAdd(FileEvent &fileEvent);//添加事件
    void EpollMod(FileEvent &fileEvent);//修改事件
    void EpollDel(FileEvent &fileEvent);//删除事件
    std::vector<struct epoll_event> EpollDispatch(int timeout);//事件分发,阻塞时间由定时器事件决定
    int GetEpollFd();//获取epoll的fd
private:
    Epoll(Epoll &rth);//禁止拷贝
    Epoll &operator=(Epoll &rth);//禁止赋值
    static const int MAXFDS = 1000;//假如epoll管理的最大fd个数
    static const int EPOLLWAIT_TIME = 1000;//默认阻塞1s,为了比较及时处理定时器事件
    int epollFd_;
    std::vector<struct epoll_event> events_;//返回就绪事件列表
};

#endif // EPOLL_H
