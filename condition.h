#ifndef CONDITION_H
#define CONDITION_H

#include"mutexlock.h"
#include<pthread.h>
#include<time.h>
#include<errno.h>
/*
封装Linux条件变量对应的API
*/

class Condition{
public:
    //explicit禁止构造函数隐式转换
    /*
        void test(Condition &cond);//圆形
        MutexLock mutex;
        test(mutex);//如何没有explicit,那么编译器将隐式将首先定义临时Condition对象,然后通过mutex初始化.
        此处禁止这种操作,因为不需要这种操作
*/
    explicit Condition(MutexLock &mutex):mutex_(mutex){
        pthread_cond_init(&cond_ , NULL);
    }

    ~Condition(){
        pthread_cond_destroy(&cond_);
    }

    void wait(){//等待条件满足,并等待另外一个线程唤醒
        pthread_cond_wait(&cond_ , mutex_.get() );
    }

    void notify(){//单播,唤醒一个等待该条件的线程
        pthread_cond_signal(&cond_);
    }

    void notifyALL(){//广播,唤醒全部等待该条件的线程
        pthread_cond_broadcast(&cond_);
    }
    bool waitForSeconds(double seconds){//超时等待,可设置4.67s,超时则等待返回
        struct timespec realTimeClock;
        clock_gettime(CLOCK_REALTIME , &realTimeClock);//获取当前时间
        int64_t totalNsecond = static_cast<int64_t>(realTimeClock.tv_nsec + seconds*1000000000);
        realTimeClock.tv_sec += static_cast<__time_t>(totalNsecond/1000000000);
        realTimeClock.tv_nsec = static_cast<__suseconds_t>(totalNsecond%1000000000);
        return ETIMEDOUT == pthread_cond_timedwait(&cond_ , mutex_.get() , &realTimeClock);
        //如果距离当前时间n秒后等待超时返回则true,否则是false.
    }

private:
     MutexLock &mutex_;//传递引用,为了将互斥锁和条件变量解耦分开来.
     pthread_cond_t cond_;//条件变量对应的变量
     Condition(const Condition& rhs);//禁止拷贝
     Condition& operator=(const Condition& rhs);//禁止赋值
};


#endif // CONDITION_H
