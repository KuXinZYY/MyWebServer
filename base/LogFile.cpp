#include "LogFile.h"

LogFile::LogFile(const std::string &basename , int autoFlushEveryN)
    : basename_(basename),
      autoFlushEveryN_(autoFlushEveryN),
      count_(0),
      mutex_(),
      file_(basename_)
{

}
LogFile::~LogFile()
{

}

void LogFile::append(const char *logline , const size_t len)
{
    MutexLockGuard lock(mutex_);
    file_.append(logline , len);
    ++count_;
    if(autoFlushEveryN_ < count_){
        count_ = 0;
        file_.flush();
    }
}
void LogFile::flush()
{
    MutexLockGuard lock(mutex_);
    file_.flush();
}

