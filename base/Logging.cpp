#include "Logging.h"
#include<memory>
static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static std::shared_ptr<AsynLogging> AsyncLogger_;//静态对象


/*
    void formatTime();
    LogStream stream_;// <<重载数据流
    int line_;
    std::string logFileName_;//Log文件名字
*/

/*
通过RALL封装,
1.构造的时候初始化异步日志.
2.析构的时候,将日志写入文件.
*/
void once_init()
{
    AsyncLogger_ = std::make_shared<AsynLogging>( Logger::getLogFileName() , 40 );//后台日志线程
    AsyncLogger_->start();//后台线程开始运行
}

void output(const char* msg, int len)
{
    pthread_once(&once_control_, once_init);//保证once_init函数仅仅初始化一次,以后直接跳过
    AsyncLogger_->append(msg, len);
}


Logger::Logger(const char *fileName, int line)
  : stream_(),
    fileName_(fileName),
    line_(line)
{
    formatTime();//构造函数初始化文件名 行数 以及当前日志产生时间
}

Logger::~Logger()
{
    stream_ << " - " << fileName_ << ':' << line_ << '\n';//日期时间 - 正文 源文件名:行号
    const LogStream::Buffer &temp = stream_.buffer();
    output(temp.data() , temp.length());//将stream_中的内容缓存到logFile中
}

void Logger::formatTime()
{
    struct timeval tv;    
    time_t time;
    char str_t[26] = {0};

    gettimeofday(&tv, NULL);//获取日历时间

    time = tv.tv_sec;

    struct tm* p_time = localtime(&time);

    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S ", p_time);//时间 正文 源文件名:行号

    stream_ << str_t;
}

std::string Logger::logFileName_ = "wangjun_WebServer.log";
