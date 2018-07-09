#ifndef THREAD_H
#define THREAD_H

/*
封装pthread_create
*/
#include"CurrentThread.h"

#include<assert.h>
#include<memory.h>
#include<stdio.h>
#include<string>
#include<stddef.h>//标准定义头文件,使用NULL需要
#include<unistd.h>//包含syscall
#include<sys/syscall.h>//包含syscall的符号常量
#include<sys/prctl.h>

#include<pthread.h>
#include<string>
#include"CountDownLatch.h"
#include<functional>  //仿函数对象适配器

class Thread{
public:
    //函数对象,封装了()操作符.形参类似函数的功能.可以传入函数指针.void ()表示传入无返回值无参数
    typedef std::function<void ()> ThreadFunc;//定义类,用于绑定任何函数对象,并通过()调用

    /* 1.构造函数初始化,如果第二个名字参数没有传入,则默认通过空string初始化
     * 2.这种构造函数可以被编译器默认被用来当做隐式转换函数,必须禁止
     */
    explicit Thread(const ThreadFunc& , const std::string& name = std::string());

    ~Thread();

    void threadStart();

    bool threadJoin();

    pid_t tid() const{
        return tid_;
    }

private:
    Thread(Thread &rth);//禁止拷贝
    Thread &operator=(Thread &rth);//禁止赋值
    /*
        注意构造函数初始化顺序必须一致性
    */
    void setDefaultName();//私有主要用于设定线程名称,在构造函数中调用
    ThreadFunc func_;        //线程执行的函数
    std::string threadName_;//线程名字
    pthread_t threadID_;   //线程ID
    pid_t tid_;             //线程所属进程ID
    bool threadStarted_;   //线程是否启动
    bool threadJoined_;    //线程是否等待其他线程结束
    CountDownLatch latch_;  //计时器门栓
};





#endif // THREAD_H
