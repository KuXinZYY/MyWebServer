#include "AsynLogging.h"

AsynLogging::AsynLogging(const std::string logName , int flushInterval)
    : thread_(std::bind(&AsynLogging::threadFunc , this) , std::string("logThread") ),
      currentBuffer_(new Buffer),
      backupBuffer_(new Buffer),
      logName_(logName),
      flushInterval_(flushInterval),
      buffers_(),
      mutex_(),
      cond_(mutex_),
      latch_(1),
      running_(false)
{
    //assert(logName_.size() > 0 && logName_[0] == '/');//确保日志路径正确
    //前端缓冲区清空
    currentBuffer_->bzero();
    backupBuffer_->bzero();
    buffers_.reserve(16);//预分配16个管理缓存区的空间.
}
void AsynLogging::append(const char * logline , int len)
{
    MutexLockGuard lock(mutex_);//必须互斥
    if(currentBuffer_->avail() > len)//缓冲区可用空间大于len,则直接加入当前缓冲区
        currentBuffer_->append(logline , len);
    else{//否则将当前空间,加入buffers管理,并替换新的缓冲区
        buffers_.push_back(std::move(currentBuffer_));//将currentBuffer_对象内部数据,移动到vector对象中,避免了share_ptr对象的拷贝构造和析构.
        if(backupBuffer_){//如果备份缓冲区非空,则直接交换,为下次备份记录日志
            currentBuffer_ = std::move(backupBuffer_);//将缓冲区指向的内存复制为当前
            currentBuffer_->reset();
        }
        else{
            currentBuffer_.reset(new Buffer);//重置 当前智能指针 指向的内存
            currentBuffer_->reset();//重置内存缓冲区指针
        }
        currentBuffer_->append(logline , len);//将日志添加
        cond_.notify();//通知线程来消费
    }
}

void AsynLogging::threadFunc()
{
    assert(running_ == true);
    latch_.countdown();//运行,通知主线程开始运行

    LogFile output(logName_ , 4);//新建日志,名字可以自定义

    BufferPtr newBuffer1(new Buffer);//新建临时缓冲区1
    BufferPtr newBuffer2(new Buffer);//新建临时缓冲区2

    BufferVector buffersToWrite;//临时接管buffers_便于前端继续不阻塞写数据
    buffersToWrite.reserve(16);

    while(running_){
        {   //临界区主要用于交换内存区域,并分配临时内存
            MutexLockGuard lock(mutex_);//必须互斥
            if(buffers_.empty())//这是非典型应用,不可防止虚假唤醒.
                cond_.waitForSeconds(flushInterval_);
            //到这里,证明有数据了,可以继续处理了
            buffers_.push_back(std::move(currentBuffer_));//再次将当前缓冲区数据移入buffers管理

            currentBuffer_ = std::move(newBuffer1);//移入临时缓存,为了后续前端继续写
            buffersToWrite.swap(buffers_);//交换buffers内部的指针

            if(!backupBuffer_)//预分配backupBuffer
                backupBuffer_ = std::move(newBuffer2);
        }
        assert(!buffersToWrite.empty());//断定非空

        if(buffersToWrite.size() > 25){//这里满足的条件就是,在后端线程append的时候,前端疯狂写入很多日志,导致buffers非常大
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());//仅仅保留前2个日志,后面直接丢弃.
        }
        for(size_t i = 0 ; i < buffersToWrite.size() ; i++)//将缓冲区内部数据全部加入log日志.
            output.append(buffersToWrite[i]->data() , buffersToWrite[i]->length());
        if(buffersToWrite.size() > 2)
            buffersToWrite.resize(2);//扩展为大小2

        if(!newBuffer1){//分配缓存1
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }
        if (!newBuffer2)//分配缓存2
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.back();
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();//清空buffer
        output.flush();//flush到内存缓冲区
    }
    output.flush();//flush到内存缓冲区
}
/*
主线程直接stop子线程,为啥在缓冲区的日志没有flush到对应的文件中.
*/
