#include "Server.h"
#include"ServerConnection.h"

Server::Server(EventLoop *loop , int port , int threadNum)
    : start_(false),
      loop_(loop),
      port_(port),
      listenFd_(socket_bind_listen(port_)),
      threadNum_(threadNum),
      acceptFileEvent_(new FileEvent(loop_)),
      eventLoopThreadPool_(new EventLoopThreadPool(loop_ , threadNum_))//新建一个池子
{
    acceptFileEvent_->setFd(listenFd_);//设定事件FD
    setSocketNonBlocking(listenFd_);//设定非阻塞
}
Server::~Server()
{

}
void Server::start()//服务器启动
{
    eventLoopThreadPool_->start();//启动事件循环线程池
    acceptFileEvent_->setEvent(EPOLLIN);//可读
    acceptFileEvent_->setReadHandle(std::bind(&Server::newConnHandle , this) );//设定读事件,注意newConnHandle可以访问this中的成员变量
    loop_->AddFileEvent(acceptFileEvent_);//将监听事件加入主线程的Loop
    loop_->EventLoopProcess();
}
void Server::quit()
{
    loop_->quit();//主线程循环退出
}
void Server::newConnHandle()//循环accept,然后创建新的ServerData并管理
{
    struct sockaddr_in cliaddr;
    memset(&cliaddr , 0 , sizeof(cliaddr));
    int acceptFd = -1;
    socklen_t length = sizeof(cliaddr);
    while( (acceptFd = accept( listenFd_ ,(struct sockaddr *)&cliaddr ,  &length ) ) > 0){//只有大于0,则还有可读

        EventLoop *loop = eventLoopThreadPool_->getNextLoop();
        //打印IP地址,以及端口(网络字节序转为主机)
        LOG << "New connection from " << inet_ntoa(cliaddr.sin_addr) << ":" << ntohs(cliaddr.sin_port)
            << " , The fd is " << acceptFd;
        setSocketNonBlocking(acceptFd);//将其设定为非阻塞
        shared_ptr<ServerConnection> connd(new ServerConnection(loop , acceptFd) );//加入对应的事件循环中
        ServerConnectionManager.push_back(connd);//加入服务器管理连接
        //加入当前事件循环,注意这个对象必须被动态创建,否则生命周期退出就会被析构.
    }
}
























