#include "TimeEvent.h"

TimeEventManager::TimeEventManager()
{

}
TimeEventManager::~TimeEventManager()
{

}

void TimeEventManager::addTimeEvent(TimeEvent::CallBack TimeHandle, long long milliseconds , bool persist)
{
    std::shared_ptr<TimeEvent> new_TimeEvent(new TimeEvent(TimeHandle , milliseconds , persist) );//新建事件
    TimeEventsQueue.push(new_TimeEvent);
}

void TimeEventManager::handleExpiredEvent()
{
    while(!TimeEventsQueue.empty()){//处理全部过期的事件
        TimeEventPtr timeEventPtr = TimeEventsQueue.top();
        if(timeEventPtr->isDeleted())
            TimeEventsQueue.pop();
        else if(timeEventPtr->isValid()){//如果事件就绪,则移除
            TimeEventsQueue.pop();
            if(timeEventPtr->isPersist()){//如果是定时器事件,则继续插入堆中
                timeEventPtr->updateExpiredTime(timeEventPtr->getInterval());//更新时间事件
                TimeEventsQueue.push(timeEventPtr);
            }
        }else//如果距离最近的时间事件都没有发生,那么可以结束while循环了,后面肯定没有发生.
            break;
    }
}
long long TimeEventManager::SearchNearestTime()
{
    if(!TimeEventsQueue.empty()){
        struct timeval now;
        gettimeofday(&now, NULL);
        long long CurrentTime = now.tv_sec*1000 + now.tv_usec/1000;
        TimeEventPtr timeEventPtr = TimeEventsQueue.top();//找出最近的时间
        int Temp = timeEventPtr->getExpireTime() - CurrentTime;//最短时间和当前的差值
        if( Temp > 0)
            return Temp;
        else
            return 0;
    }else
        return -1;
}

