#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include"mutexlock.h"
#include"condition.h"

/*
利用条件变量+互斥锁实现线程门栓功能
*/
class CountDownLatch
{
public:
    //避免隐式的类型转换函数,这种构造函数都需要避免不必要的转换存在
    explicit CountDownLatch(int count);
    ~CountDownLatch();

    void wait();//计数值大于0,则等待其他人到齐

    void countdown();//计数减1,当为0,则唤醒其他等待
private:
    /*
        注意成员变量初始化顺序是通过定义的顺序执行的先初始化mutex然后cond,否则直接出错.
    */
    CountDownLatch(const CountDownLatch& rhs);//禁止拷贝
    CountDownLatch& operator=(const CountDownLatch& rhs);//禁止赋值
    MutexLock mutex_;
    Condition cond_;
    int count_;
};

#endif // COUNTDOWNLATCH_H
