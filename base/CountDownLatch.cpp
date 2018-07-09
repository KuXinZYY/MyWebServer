#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)//注意这里的构造顺序
    : mutex_(),
      cond_(mutex_),
      count_(count)
{

}

CountDownLatch::~CountDownLatch()
{

}

void CountDownLatch::wait()
{
    MutexLockGuard lock(mutex_);

    while(count_ > 0)//还未全部就绪,则给我等待
        cond_.wait();
}

void CountDownLatch::countdown()
{
    MutexLockGuard lock(mutex_);//RALL加锁
    --count_;
    if(count_ == 0)//如果准备继续则通知全部
        cond_.notifyALL();
}
