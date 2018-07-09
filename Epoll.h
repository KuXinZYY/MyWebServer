#ifndef EPOLL_H
#define EPOLL_H

//#include<epoll.h>

/*
封装事件IO复用

int epoll_create();
epoll_ctl
epoll_wait

*/
class Epoll
{
public:
    Epoll();
    ~Epoll();
    int epoll_add();//添加事件
    int epoll_mod();//修改事件
    int epoll_del();//删除事件
    int epoll_dispatch();//事件分发
    int getEpollFd()//获取epoll的fd
    {
        return epollFd_;
    }
private:
    static const int MAXFDS = 100000;//epoll管理的最大fd个数
    int epollFd_;

};

#endif // EPOLL_H
