#ifndef TIMEEVENT_H
#define TIMEEVENT_H

/*
通过最小堆封装事件事件
*/
#include<functional>
#include<memory>
#include<algorithm>
#include<queue>
#include<sys/time.h>

/*
1.将定时器事件和IO事件分开处理了.
2.后续可以考虑将timefd统一起来.
*/

class TimeEvent//某个时间事件
{
public:
    typedef std::function<void()> CallBack;//事件触发的回调函数
    TimeEvent(CallBack timeHandle , long long milliseconds , bool persist)
        : deleted_(false),
          persist_(persist),
          milliseconds_(milliseconds),
          timeHandle_(timeHandle)
    {
        updateExpiredTime(milliseconds);
    }
    ~TimeEvent(){}
    bool isDeleted()
    {
        return deleted_;
    }
    void updateExpiredTime(long long milliseconds)//更新过期时间,设定距离当前时间的绝对时间,ms
    {
        //通过获取距离1970.1.1:0.0.0的秒及微妙数来定时器操作
        struct timeval now;
        gettimeofday(&now, NULL);
        //long long expiredTime_ = now.tv_sec*1000 + now.tv_usec/1000 + milliseconds;//距离特定时间的秒数
        when_sec_ = now.tv_sec + milliseconds/1000;
        when_ms_ = now.tv_usec/1000 + milliseconds%1000;
        if(when_ms_ >= 1000){
            when_sec_++;
            when_ms_ -=1000;
        }
    }
    bool isValid()//定时时间是否到达
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        //运用短路法判断是否就绪,当前时间比注册的大，那么事件就绪了
        if( (now.tv_sec > when_sec_) ||
            (now.tv_sec == when_sec_ && (now.tv_usec/1000) >= when_ms_)){
            timeHandle_();//就绪则执行
            return true;
        }
        else
            return false;
    }
    bool isPersist()
    {
        return persist_;
    }

    void setDelete()
    {
        deleted_ = true;
    }
    long long getExpireTime()
    {
        return (when_sec_*1000 + when_ms_);//返回ms
    }
    long long getInterval()
    {
        return milliseconds_;
    }
private:
    bool deleted_;//时间事件是否被删除
    bool persist_;//是否是定时器时间 也就是可能大概每4s触发一次
    long long milliseconds_;//存储定时器时间

    long long  when_sec_;//记录距离特定时间的过期事件
    long long  when_ms_;

    CallBack timeHandle_;//时间事件处理函数
};

//用于小根堆中的比较仿函数
struct TimeCmp
{
    bool operator()(std::shared_ptr<TimeEvent> &a , std::shared_ptr<TimeEvent> &b ) const {
        return a->getExpireTime() > b->getExpireTime();
    }
};

class TimeEventManager//时间事件管理器
{
public:
    TimeEventManager();
    ~TimeEventManager();
    void addTimeEvent(TimeEvent::CallBack TimeHandle, long long milliseconds , bool persist = false);
    void handleExpiredEvent();
    long long SearchNearestTime();//返回最近的时间,通过给epoll定时使用
private:
    //存放智能指针对象
    typedef std::shared_ptr<TimeEvent> TimeEventPtr;
    std::priority_queue< TimeEventPtr , std::vector<TimeEventPtr> , TimeCmp> TimeEventsQueue;//事件优先队列
};

#endif // TIMEEVENT_H
