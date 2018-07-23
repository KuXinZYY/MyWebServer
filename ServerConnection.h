#ifndef SERVERDATA_H
#define SERVERDATA_H

/*
写多线程代码,时刻记住,哪个是在哪个线程中调用
*/

#include"FileEvent.h"
#include<memory>
#include<map>
using namespace std;
/*
 * 服务器每条连接维护的类.每条连接中需要进行HTTP协议的解析,也需要读取缓冲区数据
*/

//状态机四个状态,处理请求行 请求头 处理数据 完成
enum ProcessStatus{
    PARSE_LINE = 1,
    PARSE_HEADERS,
    RECV_BODY,
    ANALYSIS,
    FINISH
};

enum RequstLineState//解析请求行返回的错误码
{
    PARSE_URI_AGAIN = 1,
    PARSE_URI_ERROR,
    PARSE_URI_SUCCESS,
};
enum HttpMethod//HTTP服务器可解析的三种方法 get post head
{
    METHOD_POST = 1,
    METHOD_GET,
    METHOD_HEAD
};
enum HttpVersion
{
    HTTP_10 = 1,
    HTTP_11
};
enum HeadState//头状态
{
    PARSE_HEADER_SUCCESS = 1,
    PARSE_HEADER_AGAIN,
    PARSE_HEADER_ERROR
};
enum ParseState
{
    H_START = 0,
    H_KEY,
    H_COLON,
    H_SPACES_AFTER_COLON,
    H_VALUE,
    H_CR,
    H_LF,
    H_END_CR,
    H_END_LF
};
enum AnalysisState
{
    ANALYSIS_SUCCESS = 1,
    ANALYSIS_ERROR
};

class ServerConnection
{
public:
    ServerConnection(EventLoop *loop , int acceptedFd);
    ~ServerConnection();
    void handleRead();//处理此条连接的读
    void handleWrite();//处理此条连接的写
    void pushEventInEventLoopQueue();//将acceptedFdEvent添加到loop对应的队列中
private:
    void drive_machine();//有限状态机解析HTTP协议
    RequstLineState PraseRequstLine();//解析HTTP协议的请求行
    HeadState PraseRequstHead();//解析请求头部
    AnalysisState AnalysisRequest();//分析读取数据

    void WriteToClient();//将数据发送给客户端
    void HandleError(int fd, int err_num, string short_msg);//处理错误
    EventLoop *loop_;
    std::shared_ptr<FileEvent> acceptedFdEvent;//已连接的事件
    int acceptedFd_;//已连接的FD
    std::string inBuffer_;//HTTP输入缓冲
    std::string outBuffer_;//HTTP输出缓冲
    ProcessStatus state_;//HTTP协议状态机
    HttpMethod method_;//HTTP方法
    std::string fileName_;//文件名字
    HttpVersion version_;//HTTP协议版本
    ParseState hState_;//存储头部状态机
    std::map<string , string> headers_;//存储请求头部信息
    bool error_;//解析过程是否出错?
};

#endif // SERVERDATA_H
