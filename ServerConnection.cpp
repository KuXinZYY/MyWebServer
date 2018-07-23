#include "ServerConnection.h"
#include "EventLoop.h"
#include <memory>
#include <sys/mman.h>
#include <sys/stat.h>

pthread_once_t OnceInit = PTHREAD_ONCE_INIT;
static map<string , string> MIMEType;//存储MIMEType类型,方便服务发送请求报文
static void MimeTypeInit()
{
    MIMEType[".html"] = "text/html";
    MIMEType[".avi"] = "video/x-msvideo";
    MIMEType[".bmp"] = "image/bmp";
    MIMEType[".c"] = "text/plain";
    MIMEType[".doc"] = "application/msword";
    MIMEType[".gif"] = "image/gif";
    MIMEType[".gz"] = "application/x-gzip";
    MIMEType[".htm"] = "text/html";
    MIMEType[".ico"] = "image/x-icon";
    MIMEType[".jpg"] = "image/jpeg";
    MIMEType[".png"] = "image/png";
    MIMEType[".txt"] = "text/plain";
    MIMEType[".mp3"] = "audio/mp3";
    MIMEType["default"] = "text/html";
}
static string &getMimeType(const string &Key)//通过键找回值
{
    pthread_once(&OnceInit, MimeTypeInit);//确保仅仅初始化一次
    map<string , string>::iterator ite;
    ite = MIMEType.find(Key);
    if(ite == MIMEType.end())
        return MIMEType["default"];
    else
        return MIMEType[Key];
}

ServerConnection::ServerConnection(EventLoop *loop , int acceptedFd)
    : loop_(loop),
      acceptedFdEvent(new FileEvent(loop)),
      acceptedFd_(acceptedFd),
      inBuffer_(),
      outBuffer_(),
      state_(PARSE_LINE),
      method_(METHOD_GET)
{
    acceptedFdEvent->setFd(acceptedFd);//设定事件fd
    //acceptedFdEvent->setEvent(EPOLLIN);//可读
    acceptedFdEvent->setReadHandle(std::bind(&ServerConnection::handleRead , this) );//handleRead函数可以获取ServerConnection类中的信息,自动传参
    acceptedFdEvent->setWriteHandle(std::bind(&ServerConnection::handleWrite , this) );
    loop_->queueInLoop(std::bind(&ServerConnection::pushEventInEventLoopQueue , this));//将事件添加至loop循环队列中,醒来第一件事就是注册事件
    //loop_->AddFileEvent(acceptedFdEvent);
}
ServerConnection::~ServerConnection()
{

}
void ServerConnection::handleRead()//回射服务器,读取然后直接返回
{
    /*
        回显的例子
    */
//    char buff[4096] = {0};//避免valgrind报警
//    int readSum = readn(acceptedFd_, buff, 4096);
//    if(writen(acceptedFd_ , buff , readSum) != readSum)
//        abort();
    drive_machine();//直接解析HTTP协议,并返回结果

}
void ServerConnection::handleWrite()
{

}
//这个是当EventLoop醒来之后,会调用队列中的函数.这样可以简化系统的设计,类似Memcached功能.
void ServerConnection::pushEventInEventLoopQueue()//IO线程EventFd就绪调用
{
    acceptedFdEvent->setEvent(EPOLLIN);//设定事件可读事件
    loop_->AddFileEvent(acceptedFdEvent);//加入到对应的事件循环中
}

void ServerConnection::drive_machine()
{
    int readSum = readn(acceptedFd_ , inBuffer_);//将数据读取.
    LOG <<  CurrentThread::name() << "("<<CurrentThread::tid() << ") "
        << "Resqust: " << inBuffer_;//打印出读取的数据
    if (readSum <= 0)
    {
        HandleError(acceptedFd_, 400, "Bad Request");//请求错误
        perror("readSum <= 0");
        abort();
    }//状态机开始循环处理
    error_ = false;//每次进来标记错误为假
    state_ = PARSE_LINE;
    while(state_ != FINISH && !error_){//状态机开始解析HTTP协议
        switch(state_){
            case PARSE_LINE:
            {
                RequstLineState flag = PraseRequstLine();//解析行
                if (flag == PARSE_URI_AGAIN)
                    break;
                else if (flag == PARSE_URI_ERROR)
                {
                    perror("2");
                    LOG << "FD = " << acceptedFd_ << "," << inBuffer_ << "******";
                    inBuffer_.clear();
                    error_ = true;
                    HandleError(acceptedFd_, 400, "Bad Request");
                    break;
                }
                else
                    state_ = PARSE_HEADERS;//成功则继续解析请求头
                break;
            }

            case PARSE_HEADERS:
            {
                HeadState flag = PraseRequstHead();
                if (flag == PARSE_HEADER_AGAIN)
                    break;
                else if (flag == PARSE_HEADER_ERROR)
                {
                    perror("3");
                    error_ = true;
                    HandleError(acceptedFd_, 400, "Bad Request");
                    break;
                }
                if(method_ == METHOD_POST)
                {
                    // POST方法准备
                    state_ = RECV_BODY;//post则接受
                }
                else
                {
                    state_ = ANALYSIS;//get则分析然后返回
                }
                break;
            }
            case RECV_BODY:
            {
                int content_length = -1;
                if (headers_.find("Content-length") != headers_.end())//找到长度信息
                {
                    content_length = stoi(headers_["Content-length"]);
                }
                else
                {
                    error_ = true;
                    HandleError(acceptedFd_, 400, "Bad Request: Lack of argument (Content-length)");
                    break;
                }
                if (static_cast<int>(inBuffer_.size()) < content_length)
                    break;
                state_ = ANALYSIS;
                break;
            }
            case ANALYSIS:
            {
                AnalysisState flag = AnalysisRequest();
                if (flag == ANALYSIS_SUCCESS)
                {
                    state_ = FINISH;
                    break;
                }
                else
                {
                    //cout << "state_ == STATE_ANALYSIS" << endl;
                    error_ = true;
                    break;
                }
                break;
            }
            case FINISH:
                break;
        }
    }

}
RequstLineState ServerConnection::PraseRequstLine()
{
    string &str = inBuffer_;
    // 读到完整的请求行再开始解析请求
    size_t pos = str.find('\r', 0);//从字符串开始,找到第一个回车符的位置
    if(pos < 0){//如果没有找到,则数据不完整,重新读取
        return PARSE_URI_AGAIN;
    }
    string RequstLine = str.substr(0, pos);//分离出请求行

    //支持的方法,通过find三种识别方法出来
    int posGet = RequstLine.find("GET");
    int posPost = RequstLine.find("POST");
    int posHead = RequstLine.find("HEAD");

    if (posGet >= 0)//标记方法
    {
        pos = posGet;
        method_ = METHOD_GET;
    }
    else if (posPost >= 0)
    {
        pos = posPost;
        method_ = METHOD_POST;
    }
    else if (posHead >= 0)
    {
        pos = posHead;
        method_ = METHOD_HEAD;
    }
    else
    {
        return PARSE_URI_ERROR;
    }
    //找文件名
    pos = RequstLine.find("/", pos);//找到请求行中第一个/的初始位置
    if (pos < 0)//找不到,则使用默认文件
    {
        fileName_ = "index.html";
        version_ = HTTP_11;
        return PARSE_URI_SUCCESS;
    }
    else//否则找到第一个空格位置,然后截取文件名
    {
        size_t _pos = RequstLine.find(' ', pos);
        if (_pos < 0)
            return PARSE_URI_ERROR;
        else
        {
            if (_pos - pos > 1)
            {
                fileName_ = RequstLine.substr(pos + 1, _pos - pos - 1);
                size_t __pos = fileName_.find('?');
                if (__pos >= 0)
                {
                    fileName_ = fileName_.substr(0, __pos);
                }
            }
            else
                fileName_ = "index.html";
        }
        pos = _pos;
    }
    //cout << "fileName_: " << fileName_ << endl;
    //找出HTTP的版本号
    pos = RequstLine.find("/", pos);//找到第二个/,也就是HTTP协议版本
    if (pos < 0)
        return PARSE_URI_ERROR;
    else
    {
        if (RequstLine.size() - pos <= 3)
            return PARSE_URI_ERROR;
        else
        {
            string ver = RequstLine.substr(pos + 1, 3);
            if (ver == "1.0")
                version_ = HTTP_10;
            else if (ver == "1.1")
                version_ = HTTP_11;
            else
                return PARSE_URI_ERROR;
        }
    }
    return PARSE_URI_SUCCESS;
}
HeadState ServerConnection::PraseRequstHead()//解析请求头
{
    string &str = inBuffer_;
    int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
    int now_read_line_begin = 0;
    bool notFinish = true;
    size_t i = 0;
    hState_ = H_START;//状态机开始
    for (; i < str.size() && notFinish; ++i)//找出全部的请求头
    {
        switch(hState_)
        {
            case H_START:
            {
                if (str[i] == '\n' || str[i] == '\r')//开头不可能为空,则证明找玩了
                    break;
                hState_ = H_KEY;
                key_start = i;//记录键开始位置
                now_read_line_begin = i;
                break;
            }
            case H_KEY://找键
            {
                if (str[i] == ':')
                {
                    key_end = i;//找到空格,则记录键结束位置
                    if (key_end - key_start <= 0)
                        return PARSE_HEADER_ERROR;
                    hState_ = H_COLON;//进入找值状态
                }
                else if (str[i] == '\n' || str[i] == '\r')
                    return PARSE_HEADER_ERROR;
                break;
            }
            case H_COLON:
            {
                if (str[i] == ' ')
                {
                    hState_ = H_SPACES_AFTER_COLON;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;
            }
            case H_SPACES_AFTER_COLON:
            {
                hState_ = H_VALUE;
                value_start = i;
                break;
            }
            case H_VALUE:
            {
                if (str[i] == '\r')
                {
                    hState_ = H_CR;
                    value_end = i;
                    if (value_end - value_start <= 0)
                        return PARSE_HEADER_ERROR;
                }
                else if (i - value_start > 255)
                    return PARSE_HEADER_ERROR;
                break;
            }
            case H_CR:
            {
                if (str[i] == '\n')
                {
                    hState_ = H_LF;
                    string key(str.begin() + key_start, str.begin() + key_end);
                    string value(str.begin() + value_start, str.begin() + value_end);
                    headers_[key] = value;
                    now_read_line_begin = i;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;
            }
            case H_LF:
            {
                if (str[i] == '\r')
                {
                    hState_ = H_END_CR;
                }
                else
                {
                    key_start = i;
                    hState_ = H_KEY;
                }
                break;
            }
            case H_END_CR:
            {
                if (str[i] == '\n')
                {
                    hState_ = H_END_LF;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;
            }
            case H_END_LF:
            {
                notFinish = false;
                key_start = i;
                now_read_line_begin = i;
                break;
            }
        }
    }
    if (hState_ == H_END_LF)
    {
        str = str.substr(i);
        return PARSE_HEADER_SUCCESS;
    }
    str = str.substr(now_read_line_begin);
    return PARSE_HEADER_AGAIN;
}
AnalysisState ServerConnection::AnalysisRequest()
{
    if (method_ == METHOD_POST)
    {
        //post暂时不处理.
    }
    else if (method_ == METHOD_GET || method_ == METHOD_HEAD)//仅仅处理GET和HEAD
    {
        string header;
        header += "HTTP/1.1 200 OK\r\n";//响应头
        if(headers_.find("Connection") != headers_.end() && (headers_["Connection"] == "Keep-Alive" || headers_["Connection"] == "keep-alive"))
        {
            //keepAlive_ = true;
            //header += string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" + to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
        }
        int dot_pos = fileName_.find('.');
        string filetype;
        if (dot_pos < 0)
            filetype = getMimeType("default");
        else
            filetype = getMimeType(fileName_.substr(dot_pos));//返回文件类型
        //回显测试
        if (fileName_ == "hello")
        {
            outBuffer_ = "HTTP/1.1 200 OK\r\nContent-type: text/plain\r\n\r\nHello World";
            return ANALYSIS_SUCCESS;
        }
//        if (fileName_ == "favicon.ico")
//        {
//            header += "Content-Type: image/png\r\n";
//            header += "Content-Length: " + to_string(sizeof favicon) + "\r\n";
//            header += "Server: LinYa's Web Server\r\n";

//            header += "\r\n";
//            outBuffer_ += header;
//            outBuffer_ += string(favicon, favicon + sizeof favicon);;
//            return ANALYSIS_SUCCESS;
//        }

        struct stat sbuf;//否则打开文件内容,然后发送出去
        if (stat(fileName_.c_str(), &sbuf) < 0)
        {
            header.clear();
            HandleError(acceptedFd_, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        header += "Content-Type: " + filetype + "\r\n";
        header += "Content-Length: " + to_string(sbuf.st_size) + "\r\n";
        header += "Server: My WebServer\r\n";
        // 头部结束
        header += "\r\n";
        outBuffer_ += header;

        if (method_ == METHOD_HEAD)
            return ANALYSIS_SUCCESS;

        int src_fd = open(fileName_.c_str(), O_RDONLY, 0);//读取
        if (src_fd < 0)
        {
          outBuffer_.clear();
          HandleError(acceptedFd_, 404, "Not Found!");
          return ANALYSIS_ERROR;
        }
        void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);//读取数据
        close(src_fd);//关闭
        if (mmapRet == (void *)-1)
        {
            munmap(mmapRet, sbuf.st_size);
            outBuffer_.clear();
            HandleError(acceptedFd_, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        char *src_addr = static_cast<char*>(mmapRet);
        outBuffer_ += string(src_addr, src_addr + sbuf.st_size);;
        munmap(mmapRet, sbuf.st_size);//搞定
        return ANALYSIS_SUCCESS;
    }
    return ANALYSIS_ERROR;
}

void ServerConnection::WriteToClient()//将outbuffer中数据发送给客户端
{
        if (writen(acceptedFd_, outBuffer_) < 0)
        {
            perror("writen");
//            events_ = 0;
            error_ = true;
        }
        //if (outBuffer_.size() > 0)//非阻塞的写,当接受缓冲区已满,则必须监听,然后移除
            //events_ |= EPOLLOUT;
}
void ServerConnection::HandleError(int fd, int err_num, string short_msg)
{
    short_msg = " " + short_msg;
    char send_buff[4096];
    string body_buff, header_buff;

    body_buff += "<html><title>error</title>";
    body_buff += "<body bgcolor=\"ffffff\">";
    body_buff += to_string(err_num) + short_msg;
    body_buff += "<hr><em> MyWebServer</em>\n</body></html>\r";

    header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";//状态行
    header_buff += "Content-Type: text/html\r\n";//消息报头1
    header_buff += "Connection: Close\r\n";//消息报头2
    header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
    header_buff += "Server: MyWebServer\r\n";;//消息报头3
    header_buff += "\r\n";//空白行

    // 错误处理不考虑writen不完的情况
    sprintf(send_buff, "%s", header_buff.c_str());
    writen(fd, send_buff, strlen(send_buff));//先发送头
    sprintf(send_buff, "%s", body_buff.c_str());
    writen(fd, send_buff, strlen(send_buff));//再正文
}

