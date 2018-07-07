#ifndef LOGGING_H
#define LOGGING_H
#include"logstream.h"
#include"asynlogging.h"
#include<sys/time.h>

class Logger
{
public:
    //当前文件以及对应的行
    Logger(const char *fileName, int line);

    ~Logger();

    LogStream& stream()
    {
        return stream_;
    }

    /*
        下面两个必须是静态的,因为可以使用外部设定名字
    */
    static void setLogFileName(std::string fileName)
    {
        logFileName_ = fileName;
    }

    static std::string getLogFileName()
    {
        return logFileName_;
    }

private:
    void formatTime();

    LogStream stream_; // << 重载数据流,构造函数将日志写入stream_ , 析构函数将日志从stream_写入LogFile文件
    std::string fileName_;//这条日志所在的文件名
    int line_;//这条日志所在的文件名对应的行数
    //上述三个变量,全部临时对象独有

    static std::string logFileName_;//Log日志名字,全部公用
};

#define LOG Logger(__FILE__, __LINE__).stream()//产生一个临时对象,操作完毕立马析构,通过这种模式简化写入LogFile文件的操作

#endif // LOGGING_H
