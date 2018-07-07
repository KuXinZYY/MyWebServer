#include"thread.h"
#include<stdio.h>
#include<binders.h>
#include<threadpool.h>
#include<fileutil.h>
/*
 * 测试线程创建


void threadFunc()//获取线程tid
{
  printf("tid = %d , threadname = %s\n", CurrentThread::tid() , CurrentThread::name());
}

void threadFunc2(int x)//获取tid,并打印传入参数x
{
  printf("tid = %d , x = %d , threadname = %s\n", CurrentThread::tid(), x,CurrentThread::name());
}

void threadFun3()
{
    printf("tid=%d\n" , CurrentThread::tid());
    sleep(1);
}


//包装简单的成员函数

class Foo{
public:
    explicit Foo(int x)
        : x_(x){

    }
    void memberFunc1(){
        printf("tid = %d , Foo::x = %f\n" , CurrentThread::tid() , x_);
    }
    void memberFunc2(const std::string & input){
        printf("tid = %d , Foo::x = %f , text = %s\n" , CurrentThread::tid() , x_ , input.c_str());
    }
private:
    float x_;
};

int main()
{
    Thread thread1(threadFunc , "thread1");
    thread1.threadStart();
    thread1.threadJoin();

    Thread thread2(std::bind(threadFunc2 , 42) , "thread2");//bind用于将threadFunc2参数绑定为42,当调用的时候,参数为42
    thread2.threadStart();
    thread2.threadJoin();

    Foo foo(0.25);

    Thread thread3(std::bind(&Foo::memberFunc1 , &foo) , "thread3");//绑定foo的成员函数
    thread3.threadStart();
    thread3.threadJoin();

    Thread thread4(std::bind(&Foo::memberFunc2 , &foo , std::string("thread4 test") ) , "thread4");//绑定foo的成员函数
    thread4.threadStart();
    thread4.threadJoin();

    auto x = std::bind(&Foo::memberFunc2 , &foo , std::placeholders::_1);
    x(std::string("wangjun"));
    return 0;
}
*/

/*
 * 测试线程池

void threadFunc()//获取线程tid
{
  printf("tid = %d , threadname = %s\n", CurrentThread::tid() , CurrentThread::name());
  sleep(1);
}
int main(void)
{
    ThreadPool MyPool("ThreadPool");

    MyPool.initThreadPool(10 , 10);

    for(int i = 0 ; i < 10 ; i++){
        MyPool.addTask(threadFunc);
    }

    sleep(4);

    MyPool.stopThreadPool();

    return 0;
}
*/
/*
 * 测试fileutil底层封装

int main(void)
{
    AppendFile logfile("log");//创建或者打开log.

    string loginfo = "log1\n";
    logfile.append(loginfo.c_str() , loginfo.length());//尾部添加log1\n

    loginfo = "log2\n";
    logfile.append(loginfo.c_str() , loginfo.length());//尾部添加log2\n

    loginfo = "log3\n";
    logfile.append(loginfo.c_str() , loginfo.length());//尾部添加log2\n

    logfile.flush();//手动将用户缓冲区冲刷到内核缓冲区.

    return 0;
}
*/

/*
测试 Log
*/
//#include"logfile.h"
//int main(void)
//{
//    LogFile logfile(std::string("log") , 10);//创建或者打开log.

//    string loginfo = "log1\n";
//    logfile.append(loginfo.c_str() , loginfo.length());//尾部添加log1\n

//    loginfo = "log2\n";
//    logfile.append(loginfo.c_str() , loginfo.length());//尾部添加log2\n

//    loginfo = "log3\n";
//    logfile.append(loginfo.c_str() , loginfo.length());//尾部添加log2\n

//    //logfile.flush();//手动将用户缓冲区冲刷到内核缓冲区.

//    return 0;
//}
/*
测试LOG宏定义
*/
#include"logging.h"
#include<iostream>
void threadFunc()
{
    for (int i = 0; i < 100000; ++i)
    {
        LOG << i;
    }
}
int main(void)
{
    //测试类型重载   17 lines
    std::cout << "----------type test-----------" << std::endl;

    Logger::setLogFileName(std::string("Test.log"));

    LOG << 0;
    LOG << 1234567890123;
    LOG << 1.0f;
    LOG << 3.1415926;
    LOG << (short) 1;
    LOG << (long long) 1;
    LOG << (unsigned int) 1;
    LOG << (unsigned long) 1;
    LOG << (long double) 1.6555556;
    LOG << (unsigned long long) 1;
    LOG << 'c';
    LOG << "abcdefg";
    LOG << "fdfefg";
    LOG << "fdfdfdefg";
    LOG << "abfdffg";
    LOG << "aFdfdefg";
    LOG << string("This is a string");//注意临时对象属于右值,不允许修改其值,因此必须使用常量引用.

    //单线程测试  100000 lines
//    cout << "----------stressing test single thread-----------" << endl;
//    for (int i = 0; i < 100000; ++i)
//    {
//        LOG << i;
//    }
    /*
     * 通过awk获取行数,正确
      awk 'END {print NR}' ./Test.log
       100017
    */


    //多线程测试  10个线程,每个线程10000行  threadNum * 100000 lines
//    cout << "----------stressing test multi thread-----------" << endl;
//    vector<shared_ptr<Thread>> threads;
//    for (int i = 0; i < 10; ++i)
//    {
//        threads.push_back(make_shared<Thread>(threadFunc , "testFunc"));
//    }
//    for (int i = 0; i < 10; ++i)
//    {
//        threads[i]->threadStart();
//    }
//    for (int i = 0; i < 10; ++i)
//    {
//        threads[i]->threadJoin();
//    }

    /*
    awk 'END {print NR}' ./Test.log
        1100017  正确
    */

    //sleep(3);
/*
线程结束,为什么没有将内核缓冲区的数据同步到文件.
*/
    return 0;
}







