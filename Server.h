#ifndef SERVER_H
#define SERVER_H

#include"FileEvent.h"
#include<functional>
#include<memory>
#include"EventLoop.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include"Util.h"
#include"EventLoopThreadPool.h"
#include"ServerConnection.h"
#include<memory>
#include<vector>
using namespace std;
class Server
{
public:
    Server(EventLoop *loop , int port , int threadNum);
    ~Server();
    void start();
    void quit();
    void newConnHandle();
private:
    bool start_;
    EventLoop *loop_;//主事件循环
    int port_;
    int listenFd_;
    int threadNum_;
    std::shared_ptr<FileEvent> acceptFileEvent_;//接收事件
    vector<shared_ptr<ServerConnection>> ServerConnectionManager;//ServerConnection的引用计数都是1
    std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;//线程池,通过轮询问执行事件分发,引用计数为1
};

#endif // SERVER_H
