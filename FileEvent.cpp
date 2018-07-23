#include"FileEvent.h"

#include"EventLoop.h"
FileEvent::FileEvent(EventLoop *loop)
    : loop_(loop),
      fd_(-1),
      event_(0),
      revent_(0),
      lastEvent_(event_),
      readHandle_(NULL),
      writeHandle_(NULL),
      errorHandle_(NULL)
{

}
FileEvent::~FileEvent()
{

}
void FileEvent::handleEvent()
{
    if(revent_ & (EPOLLIN | EPOLLPRI | EPOLLHUP) ){//当可读取 对方挂断,则调用读回调
        if(readHandle_)
            readHandle_();//可读则执行读回调
    }
    if(revent_ & EPOLLOUT){
        if(writeHandle_)
            writeHandle_();//可写则执行写回调
    }
    if(revent_ & EPOLLERR){
        if(errorHandle_)
            errorHandle_();//错误则执行错误回调
    }
}

