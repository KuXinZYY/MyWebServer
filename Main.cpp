#include"base/Thread.h"
#include<stdio.h>
#include<binders.h>
#include"base/ThreadPool.h"
#include"base/FileUtil.h"
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

#include"base/Logging.h"
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

 //通过awk获取行数,正确
 //     awk 'END {print NR}' ./Test.log
 //      100017



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


//    awk 'END {print NR}' ./Test.log
//        1100017  正确


    //sleep(3);

//线程结束,为什么没有将内核缓冲区的数据同步到文件.

    return 0;
}*/
/*
 * 测试单线程的回射服务器.
 * 添加后台运行模式.
*/
#include"EventLoop.h"
#include<iostream>
#include"FileEvent.h"
#include"Server.h"
#include<sys/types.h>
#include<sys/stat.h>
void timeHandle1()
{
    std::cout << "time 1000 test" << std::endl;
}
void timeHandle2()
{
    std::cout << "time 10000 test" << std::endl;
}
void HandleAccept(int fd)
{

}
void HandleRead(int fd)
{

}
EventLoop MainLoop;
static void sigHandler(int sigint)//接收到中断信号,直接返回终止
{
    if(sigint == SIGINT){
        printf("Received SIGINT\n");
        MainLoop.quit();//退出,休眠2s,然后会调用Server的析构函数,然后以此调用线程的回调函数,进而优雅的退出
    }
}
int daemonize(int nochdir, int noclose)
{
    int fd;
/*
调用fork产生一个子进程，同时父进程退出。我们所有后续工作都在子进程中完成。
这样做的目的在于：

1、如果我们是从命令行执行的该程序，这可以造成程序执行完毕的假象，shell会正常返回。

2、虽然子进程继承了父进程的进程组ID，但获得了一个新的进程ID，这就保证了子进程不是一个进程组的组长进程。
   保证setsid执行可成功产生新的会话。 因为组长进程ID等于其进程ID。

3、由于父进程已经先于子进程退出，会造成子进程  变成一个孤儿进程（orphan），并被Init 1号进程收养了。


*/

    switch (fork()) {
    case -1:
        return (-1);//出错直接返回
    case 0:
        break;//子进程继续向下执行
    default:
        _exit(EXIT_SUCCESS);//父进程则直接退出
    }
/*
调用setsid系统调用。这是整个过程中最重要的一步。
创建一个新的会话（session），并任此进程为该会话的组长（session leader）
如果调用进程是一个进程组的组长，调用就会失败，但这已经在第1步得到了保证。
调用setsid有3个作用：让进程摆脱原进程的控制。
    让进程摆脱原会话的控制；
    让进程摆脱原进程组的控制；
    让进程摆脱原控制终端的控制；
    就是让进程完全独立出来，脱离所有其他进程的控制。
*/
    if (setsid() == -1)//设置一个新的会话，成为首进程、成为组长进程、没有控制终端。
        return (-1);

/*
把当前工作目录切换到根目录。因为从父进程处继承过来的当前工作目录可能在一个挂载的
文件系统中。因为守护进程通常在系统在引导之前是一直存在的，所以如果守护进程的当前工作
目录在一个挂载文件系统中，那么该文件系统就不能被卸载。

*/

    if (nochdir == 0) {
        if(chdir("/") != 0) {
            perror("chdir");
            return (-1);
        }
    }

/*
设置文件权限掩码。将其设置为0，表示不屏蔽任何权限，用户可以进行任何操作。

因为每个进程都会从父进程那里继承 一个文件权限掩码，当创建新文件时，这个掩码被用于设定文件的默认访问权限，
屏蔽掉某些权限，如一般用户的写权限。当另一个进程用exec调用我们编写的daemon程序时，由于我们不知道那个进程
的文件权限掩码是什么，这样在我们创建新文件时，就会带来一些麻烦。所以，我们应该重新设置文
件权限掩码，我们可以设成任何我们想要的值，但一般情况下，大家都把它设为0，这样，它就不会
屏蔽用户的任何操作。
如果你的应用程序根本就不涉及创建新文件或是文件访问权限的设定，你也完全可以把文件权限
掩码一脚踢开，跳过这一步。关闭所有不需要的文件。同文件权限掩码一样，我们的新进程会从父
进程那里继承一些已经打开了的文件。这些被打开的文件可能永远不被我们的daemon进程读或写，
但它们一样消耗系统资源，而且可能导致所在的文件系统无法卸下。需要指出的是，文件描述符为0、1和2
的三个文件，也就是我们常说的输入、输出和报错这三个文件也需要被关闭。
很可能不少读者会对此感到奇怪，难道我们不需要输入输出吗？但事实是，在上面的第2步
后，我们的daemon进程已经与所属的控制终端失去了联系，我们从终端输入的字符不可能达到daemon进程，
daemon进程用常规的方法（如printf）输出的字符也不可能在我们的终端上显示出来。所以这三个文件已经
失去了存在的价值，也应该被关闭。

*/
    // 设置文件权限掩码
     umask(0) ; // 这里原本没有，只是为了上面的基本流程说明加上的


    ///dev/null 是 Unix/Linux 里的无底洞设备
    if (noclose == 0 && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
        if(dup2(fd, STDIN_FILENO) < 0) {//将标准输入重定向到fd。
            perror("dup2 stdin");
            return (-1);
        }
        if(dup2(fd, STDOUT_FILENO) < 0) {//将标准输出重定向到fd。
            perror("dup2 stdout");
            return (-1);
        }
        if(dup2(fd, STDERR_FILENO) < 0) {//将标准错误重定向到fd。
            perror("dup2 stderr");
            return (-1);
        }

        if (fd > STDERR_FILENO) {//并关闭fd，节约资源。
            if(close(fd) < 0) {
                perror("close");
                return (-1);
            }
        }
    }
    return (0);
}
int main(void)
{
    //daemonize(0, 0);
    Logger::setLogFileName(std::string("MyWebServer.log"));
    signal(SIGINT , sigHandler);
    Server MyEcho(&MainLoop , 8888 , 4);
    MyEcho.start();
    sleep(2);//休眠2s,Server自动析构全部内存.
    return 0;
    //全程通过valgrind检测,并无内存泄露,以及段错误发生.
}
/*

MainLoop.quit()->主线程退出->执行sleep(2)->执行主线程析构函数

析构函数:

*/









