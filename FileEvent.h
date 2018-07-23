#ifndef EVENTCHANNEL_H
#define EVENTCHANNEL_H

#include<functional>
#include<sys/epoll.h>

class EventLoop;
/*
封装socket 读 写事件,这个稍微复杂点
FileEvent直接使用数组存储即可
*/

class FileEvent
{
public:
    typedef std::function<void()> CallBack;//事件触发的回调函数
    FileEvent(EventLoop *loop);
    ~FileEvent();
    bool EqualAndUpdateLastEvents()//保存上一次事件,防止重复添加
    {
        bool ret = (lastEvent_ == event_);
        lastEvent_ = event_;
        return ret;
    }
    void setReadHandle(const CallBack &readHandle)
    {
        if(readHandle)
            readHandle_ = readHandle;
    }
    void setWriteHandle(const CallBack &writeHandle)
    {
        if(writeHandle)
            writeHandle_ = writeHandle;
    }
    int getFd()
    {
        return fd_;
    }
    uint32_t getEvent()
    {
        return event_;
    }
    void setRevent(uint32_t revent)
    {
        revent_ = revent;
    }
    void rest()//表示此fd对任何事件都不感兴趣
    {
        event_ = event_ &(~event_);//清空了,表示这个fd暂时没用了
    }
    void setEvent(uint32_t event)
    {
        event_ = event;
    }
    void setFd(int fd)
    {
        fd_ = fd;
    }
    void handleEvent();
private:
    EventLoop *loop_;//事件属于的loop_,可控制
    int fd_;//保存和事件有关的fd
    uint32_t event_;//fd对应感兴趣的事件,右用户设定
    uint32_t revent_;//就绪的掩码,右epoll设定
    uint32_t lastEvent_;//保存上次感兴趣的事件.


    CallBack readHandle_; //读就绪执行的回调函数.
    CallBack writeHandle_;//写就绪执行的回调函数.
    CallBack errorHandle_;//错误执行的回调函数.
};

#endif // EVENTCHANNEL_H
