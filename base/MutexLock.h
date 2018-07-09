#ifndef MUTEXLOCK_H
#define MUTEXLOCK_H

#include<pthread.h>
#include<stdlib.h>
/*
封装互斥锁,并通过RALL手法使用锁
*/
class MutexLock{
public:
    MutexLock(){//默认构造初始化
        pthread_mutex_init(&mutex_ , NULL);
    }

    ~MutexLock(){//析构释放
        pthread_mutex_destroy(&mutex_);
    }

    void lock(){//锁
        pthread_mutex_lock(&mutex_);
    }

    void unlock(){//解锁
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t *get(){//获取锁的地址
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;//互斥锁
    MutexLock(const MutexLock& rhs);//禁止拷贝
    MutexLock& operator=(const MutexLock& rhs);//禁止赋值
};

/*
互斥锁类如何使用?
1.定义一个对象.
2.手动调用lock和unlock操作

为了简化lock和unlock操作,需要使用RALL手法封装.
RALL手法:某个对象构造的时候lock,析构的时候unlock.
因此需要实现一个MutexLockGuard类,私有变量是对象的引用,专门在对象生命周期类锁,生命结束接锁.编译器自动帮助我们
调用相应的函数.下面就是RALL手法的使用.

{
    MutexLockGuard Lock(mutex);//{}内部都加锁
    do someting;
    ..........
}//结束析构函数调用,释放锁

*/
class MutexLockGuard{
public:
    MutexLockGuard(MutexLock& mutex):mutex_(mutex){
        mutex_.lock();
    }
    ~MutexLockGuard(){
        mutex_.unlock();
    }
private:
    MutexLock &mutex_;//引用,需要传入初始化

    MutexLockGuard(const MutexLockGuard& rhs);//禁止拷贝
    MutexLockGuard& operator=(const MutexLockGuard& rhs);//禁止赋值
};


#endif // MUTEXLOCK_H
