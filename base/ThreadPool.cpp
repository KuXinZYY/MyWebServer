#include"ThreadPool.h"
/*
MutexLock mutex_;//互斥锁
Condition notFull_;//针对生产者
Condition notEmpty_;//针对消费者
string poolName_;//线程池名字
size_t maxTaskQueueSize_;//任务队列大小
deque<task> taskQueue_;//任务队列
*/
ThreadPool::ThreadPool(const string &poolName)
    : mutex_(),
      taskQueueNotFull_(mutex_),//注意初始化顺序
      taskQueueNotEmpty_(mutex_),
      poolName_(poolName),
      maxTaskQueueSize_(0),
      numberThreads_(0),
      taskQueue_(),
      threads_(),
      running_(false)
{

}
ThreadPool::~ThreadPool()
{
//    for(size_t i = 0 ; i < numberThreads_ ; i++){
//        delete threads_[i];//释放线程占用内存
//    }
}

void ThreadPool::initThreadPool(int numberThreads , int TaskQueueSize)
{
    assert(threads_.empty());//断定线程池为空

    maxTaskQueueSize_ = TaskQueueSize;//设定最大的任务队列
    numberThreads_ = numberThreads;
    running_ = true;
    //threads_.resize(numberThreads);//设定大小  这是重大错误,resize并初始化为0,必定出错
    for(int i = 0 ; i < numberThreads ; i++){
        char id[32] ;
        snprintf(id , sizeof(id) , "%d" , i+1);//将id转换成字符串
        //将线程压栈,并给每个线程命名.
        threads_.push_back( make_shared<Thread>( std::bind(&ThreadPool::runInThread , this) , poolName_+id) );
        threads_[i]->threadStart();//创建线程,并启动线程
    }
}

void ThreadPool::stopThreadPool()
{
    {
        MutexLockGuard lock(mutex_);
        running_ = false;//这里必须停止.
        taskQueueNotEmpty_.notifyALL();//唤醒全部线程,线程自动退出
    }
    //对于函数内每个对象,执行仿函数f(*first).
    //也就是主函数对于每个线程指向 pthread_join(ID,NULL),等待其结束.
    //for_each(threads_.begin() , threads_.end() ,
    //         std::bind(&Thread::threadJoin , std::placeholders::_1) );
    sleep(4);
    for(auto inter = threads_.begin() ; inter != threads_.end() ; inter++)
        (*inter)->threadJoin();
}

void ThreadPool::runInThread()
{
    while(running_){//循环运行，取任务，运行任务。线程池唤醒处理
        Task task = takeTask();//从任务队列获取任务,并构造初始化task对象.
        if(task){
            task();//直接运行task
        }
    }
}

/*主线程调用添加一个任务*/
void ThreadPool::addTask(const Task &task)
{
    if(threads_.empty()){//线程池空,则直接运行
        task();
    }else{
        MutexLockGuard lock(mutex_);
        //循环判断,防止虚假唤醒.
        while(taskQueue_.size() >= maxTaskQueueSize_)
            taskQueueNotFull_.wait();
        assert(taskQueue_.size() < maxTaskQueueSize_);

        taskQueue_.push_back(task);

        taskQueueNotEmpty_.notify();//通知任务队列非空,消费者可以消费了
    }
}

/*取出一个任务*/
ThreadPool::Task ThreadPool::takeTask()//多线程,消费者调用.获取任务需要同步.
{
    MutexLockGuard lock(mutex_);
    while(running_ && taskQueue_.empty()){//没有任务,则线程等待
        taskQueueNotEmpty_.wait();
    }
    Task task;
    if(!taskQueue_.empty()){
        //为什么需要加这一句?因为当任务做完,线程全部阻塞与上面的wait.但是调用stop唤醒任务,需要里面执行下面的语句.
        //队列为空,会从队列中取出无关的东西,然后task()执行,导致收到段错误.  这里将SIGSEGV的Bug修复了
        task = taskQueue_.front();//从任务队列头获取任务;
        taskQueue_.pop_front();//弹出任务
        taskQueueNotFull_.notify();//任务队列没有满,通知生产着可以生产了.
    }
    return task;//如果停止且无任务,则返回空,否则在结束之前执行一个任务.
    //这就是多线程编写需要注意的地方.
}
