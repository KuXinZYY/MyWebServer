#ifndef EVENTCHANNEL_H
#define EVENTCHANNEL_H

#include<functional>

/*
用于封装每个事件对应的回调函数以及存储每个事件信息.
*/
class EventChannel
{
public:
    typedef std::function<void()> CallBack;//事件触发的回调函数
    EventChannel();
    ~EventChannel();
private:
    int fd_;//保存和事件有关的fd
    uint32_t event_;//fd对应感兴趣的事件(读 或 写 或异常 ).
    uint32_t lastEvent_;//保存上次感兴趣的事件.
    uint32_t mask;//就绪的掩码

    CallBack readHandle; //读就绪执行的回调函数.
    CallBack writeHandle;//写就绪执行的回调函数.
    CallBack errorHandle;//错误执行的回调函数.
};

#endif // EVENTCHANNEL_H
