#ifndef LOGFILE_H
#define LOGFILE_H
#include<base/FileUtil.h>

/*
再次封装AppendFile,用于自动flush操作,并且为了多线程安全使用,通过互斥锁保护起来.
*/
class LogFile
{
public:
    LogFile(const std::string &basename , int autoFlushEveryN);
    ~LogFile();

    void append(const char *logline , const size_t len);
    void flush();
private:
    LogFile(LogFile &rth);//禁止拷贝
    LogFile &operator=(LogFile &rth);//禁止赋值

    const std::string basename_;
    const int autoFlushEveryN_;//定义多少次append之后,自动flush
    int count_;//计数
    MutexLock mutex_;//互斥锁
    AppendFile file_;//流
};

#endif // LOGFILE_H
