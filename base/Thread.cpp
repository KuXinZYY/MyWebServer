#include"Thread.h"
/*************************定义线程类全局变量**************************/
namespace CurrentThread//初始化
{
    /*
      多个线程,每个线程都需要存储其信息的独立内存空间,可以避免使用全局变量的同步.
      因此使用__thread修饰的POD变量,对于每个线程都特有一份,根本不需要同步,简化
      系统的设计.
    */
    __thread int t_cachedTid = 0;
    __thread char t_tidString[32] = {};
    __thread int t_tidStringLength = 6;
    __thread const char* t_threadName = "MainEventLoop";
}

pid_t gettid()//获取线程唯一的tid,用于通信
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

/*
辅助函数,用于缓存线程tid.
*/
void CurrentThread::cacheTid()
{
    if (t_cachedTid == 0)
    {
        t_cachedTid = gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    }
}
/**********************************************************************/


/**封装线程数据类,sturct使得定义说明符之前的成员全部是public,class则为private***/
struct ThreadData{
    typedef Thread::ThreadFunc ThreadFunc;

    ThreadFunc func_;//存储仿函数对象
    std::string name_;//存储线程名字
    pid_t* tid_;//存储进程pid的指针
    CountDownLatch* latch_;//存储锁指针,因为指向一个对象,不能重新构造

    //线程数据构造函数初始化,存储线程私有数据信息
    ThreadData(const ThreadFunc &func , const std::string &name , pid_t *tid , CountDownLatch *latch)
        : func_(func),
          name_(name),
          tid_(tid),
          latch_(latch)
    {

    }
    void runInThread()
    {
        *tid_ = CurrentThread::tid();//获取当前线程tid
        tid_ = NULL;//当前线程中指针清空
        latch_->countdown();//通知主线程,可以继续运行了,确保子线程在主线程之前运行.
        latch_ = NULL;//当前线程中指针清空
        CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();//从主线程获取当前线程名字.
        prctl(PR_SET_NAME, CurrentThread::t_threadName);//用于进程重命名，主进程、子进程使用不同的命令，便于命令ps -ef查看
        func_();//执行回调函数
        //CurrentThread::t_threadName = "finished";
    }
};

//pthread_creat初始化,传入startThread和ThreadData.
//当线程执行,obj是指向初始化传入的ThreadData.所以下面进行强制转换
//可以在线程执行体内,获取线程的信息.
void *startThread(void *obj)//返回值类型是void *指针
{
    //类似C语言的强制类型转换
    ThreadData *data = static_cast<ThreadData*>(obj);//static_cast使用,后面表达式必须加()
    data->runInThread();
    delete data;//执行完毕,释放内存
    return NULL;
}
/*******************************************************************/


/*************************Thread类内函数定义**************************/
Thread::Thread(const ThreadFunc &func , const std::string &name )
    : func_(func),
      threadName_(name),
      threadStarted_(false),
      threadJoined_(false),
      latch_(1)
{
    setDefaultName();//设定初始化名字
}

Thread::~Thread()
{
    if(threadStarted_ && !threadJoined_)
        pthread_detach(threadID_);
}

void Thread::threadStart()
{
    //其值为假（即为0），那么它先向stderr打印一条出错信息
    assert(!threadStarted_);//同一个线程不可以(创建)启动两次,但是对象可以调用两次,因此这里需要断定错误.
    threadStarted_ = true;//设置为真
    ThreadData *data = new ThreadData(func_ , threadName_ , &tid_ , &latch_);//latch_不能拷贝,否则就失效了.
    if(pthread_create(&threadID_ , NULL , startThread , data)){
        threadStarted_ = false;//启动失败,则标记为错误.
        delete data;
        return ;
    }else{
        latch_.wait();//线程创建成功,则先wait.
        assert(tid_ > 0);//
    }
}

bool Thread::threadJoin()//这由主线程调用,等待此线程终止
{
    assert(threadStarted_);
    assert(!threadJoined_);
    threadJoined_ = true;
    return pthread_join(threadID_ , NULL);
}
void Thread::setDefaultName()
{
    if(threadName_.empty())
        threadName_ = "Thread";
}
