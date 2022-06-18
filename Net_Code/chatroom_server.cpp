/*
 * @Author: MRL Liu
 * @Date: 2022-03-29 14:36:42
 * @Description: Linux的基于Socket的聊天室服务端
 * @LastEditTime: 2022-04-06 14:41:43
 * @FilePath: \C++\NetCode\linux_socket.cpp
 */
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<assert.h>
#include<unistd.h>
#include<stdlib.h>
#include<poll.h>//poll系统调用
#include<sys/epoll.h>//epoll系统调用
#include"log/logger.h"

#define USER_LIMIT 5 // 最大用户数量
//#define BUFFER_SIZE 64 // 读缓冲区的大小
#define FD_LIMIT 65535 // 文件描述符数量限制
#define DEFAULT_PORT 8000

#define MAX_EVENT_NUMBER 1024 
#define BUFFER_SIZE 10 
struct client_data{
    sockaddr_in address;//socketdizhi 
    char* write_buf;//待写到客户端的数据的位置
    char buf[BUFFER_SIZE];//从客户端读入的数据
};

int setnonblocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}


class Server_Poll{
public: 
    Server_Poll(){
        users=new client_data[FD_LIMIT];//客户端socket容器
        user_counter=0; // 当前用户数量
    }
    ~Server_Poll(){
        delete[]users; 
        close(m_listenfd);  // 关闭socket
    }
    void run(int port){
        LOG_INFO("服务端程序启动...\n");
        m_listenfd=this->CreateSocket();
        this->Bind_IPv4(port);
        this->Listen(5);
        LOG_INFO("等待客户端请求中...\n");
        this->Poll();
    }
private:
    int CreateSocket(){
        // 创建一个监听套接字
        int _socket = socket(PF_INET, SOCK_STREAM, 0);
        if(_socket == -1) {
            LOG_ERROR("创建socket出错：%s(errno:%d)\n",strerror(errno),errno);
        }
        else {
            LOG_DEBUG("创建socket成功\n");
        }
        return _socket;
    }
    void Bind_IPv4(int port){
        m_port=port;
        // 创建一个IPv4的socket地址
        struct sockaddr_in address;//socket地址
        bzero(&address, sizeof(address));//将该地址清空为0
        address.sin_family = AF_INET;//设置IPv4的地址族
        //inet_pton(AF_INET,ip,&address.sin_addr);//手动设置IP地址
        address.sin_addr.s_addr = htonl(INADDR_ANY);// 自动获取本机的IP地址并且设置
        address.sin_port = htons(m_port);//端口号
        //绑定socket地址
        int flag = 1;
        setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));//允许重用本地地址和端口
        int ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));//绑定socket地址
        if(ret == -1) {
            LOG_ERROR("绑定socket出错：%s(errno:%d)\n",strerror(errno),errno);
        }
        else {
            LOG_DEBUG("绑定socket成功\n");
        }
    }
    void Listen(int backlog){
        // 开启监听
        int ret = listen(m_listenfd, backlog);
        if(ret == -1) {
            LOG_ERROR("监听socket出错：%s(errno:%d)\n",strerror(errno),errno);
        }
        else {
            LOG_DEBUG("监听socket成功\n");
        }
    }
    void Poll(){
        //设置客户端socket的fds
        for(int i=1;i<=USER_LIMIT;++i){
            fds[i].fd=-1;// 文件描述符
            fds[i].events=0;//等待的事件
        }
        // 设置服务端socket的fds
        fds[0].fd=m_listenfd;//文件字符串
        fds[0].events=POLLIN|POLLERR;//等待事件：普通或优先带数据可读+错误事件
        fds[0].revents=0;//实际发生的事件
        // 循环检查fds数组
        while(1){
            int ret=poll(fds,user_counter+1,-1); //阻塞等待
            if(ret<0) {
                LOG_INFO("poll失败\n"); 
                break; 
            }
            // 遍历整个fds
            for(int i=0;i<user_counter+1;++i) {
                // 如果是服务端socket的可读信息
                if((fds[i].fd==m_listenfd)&&(fds[i].revents&POLLIN)){
                    LOG_INFO("连接到一个新的用户,现在有%d个用户\n",user_counter); 
                    this->_handleConnFromClient();
                }
                // 如果是socket的错误事件
                else if(fds[i].revents&POLLERR) {
                    this-> _handleError(fds[i]);
                    continue; 
                }
                // 如果是socket的关闭事件
                else if(fds[i].revents&POLLRDHUP) {
                   this->_handleCloseFromClient(i);
                }
                // 如果是socket的可读事件
                else if(fds[i].revents&POLLIN){
                    this->_handleMsgFromClient(i);
                }
                // 如果是socket的可写事件
                else if(fds[i].revents&POLLOUT) {
                    this->_handleRetranMsgToClient(fds[i]);
                }
            }
        }
    }
    void _handleConnFromClient(){
        // 非阻塞等待接受客户端的连接
        struct sockaddr_in client_address;
        socklen_t client_addrlen=sizeof(client_address);
        int connfd=accept(m_listenfd,(struct sockaddr*)&client_address,&client_addrlen);
        //connfd=accept(m_listenfd,(struct sockaddr*)NULL,NULL);
        // 如果连接失败
        if(connfd==-1){
            LOG_ERROR("连接socket出错: %s(errno: %d)\n",strerror(errno),errno);
            return;
        }
        /*如果请求太多，则关闭新到的连接*/ 
        if(user_counter>=USER_LIMIT) {
            const char*info="用户数量过多\n"; 
            LOG_INFO("%s",info); 
            send(connfd,info,strlen(info),0); 
            close(connfd); // 关闭客户端socket
        }
        else{
            /*对于新的连接，同时修改fds和users数组。前文已经提到，users[connfd]对应于 新连接文件描述符connfd的客户数据*/ 
            user_counter++;
            users[connfd].address=client_address; //保存socket地址
            setnonblocking(connfd); //设置该套接字为非阻塞
            fds[user_counter].fd=connfd; 
            fds[user_counter].events=POLLIN|POLLRDHUP|POLLERR; //
            fds[user_counter].revents=0; 
            LOG_INFO("连接到一个新的用户,现在有%d个用户\n",user_counter); 
        }
    }
    void _handleCloseFromClient(int i){
        /*如果客户端关闭连接，则服务器也关闭对应的连接，并将用户总数减1*/ 
        int connfd=fds[i].fd;
        users[connfd]=users[fds[user_counter].fd];
        close(connfd); 
        fds[i]=fds[user_counter]; 
        i--; 
        user_counter--; 
        printf("一个客户端主动断开了连接\n"); 
    }
    void _handleMsgFromClient(int i){
        int connfd=fds[i].fd; 
        memset(users[connfd].buf,'\0',BUFFER_SIZE); 
        int ret=recv(connfd,users[connfd].buf,BUFFER_SIZE-1,0); 
        printf("获取%d字节的客户端（%d）数据：%s\n",ret,connfd,users[connfd].buf); 
        if(ret<0) {/*如果读操作出错，则关闭连接*/ 
            if(errno!=EAGAIN) {
                close(connfd); 
                users[fds[i].fd]=users[fds[user_counter].fd]; 
                fds[i]=fds[user_counter];
                i--; 
                user_counter--; 
            }
        }
        else if(ret>0)  {/*如果接收到客户数据，则通知其他socket连接准备写数据*/ 
            for(int j=1;j<=user_counter;++j) {
                if(fds[j].fd==connfd) {continue; }
                fds[j].events|=~POLLIN; 
                fds[j].events|=POLLOUT; 
                users[fds[j].fd].write_buf=users[connfd].buf; 
            }
        }
    }
    void _handleRetranMsgToClient(pollfd fds){
        int connfd=fds.fd; 
        if(!users[connfd].write_buf) {return; }
        send(connfd,users[connfd].write_buf,strlen(users[connfd].write_buf),0); 
        users[connfd].write_buf=NULL; 
        /*写完数据后需要重新注册fds[i]上的可读事件*/ 
        fds.events|=~POLLOUT; 
        fds.events|=POLLIN; 
    }
    void _handleError(pollfd fds){
        LOG_INFO("get an error from%d\n",fds.fd); 
        char errors[100]; 
        memset(errors,'\0',100); 
        socklen_t length=sizeof(errors); 
        if(getsockopt(fds.fd,SOL_SOCKET,SO_ERROR,&errors, &length)<0) {
            LOG_INFO("get socket option failed\n"); 
        }
    }
private:
    int m_port;//端口号
    int  m_listenfd;//服务端server
    int user_counter; // 当前用户数量
    client_data* users;// 用户对象数组
    pollfd fds[USER_LIMIT+1];// poll监控的文件描述符数组，+1是因为第0个监控m_listenfd
};

class Server_Epoll{
public: 
    Server_Epoll(){
    }
    ~Server_Epoll(){
        close(m_listenfd);  // 关闭socket
    }
    void run(int port){
        LOG_INFO("服务端程序启动...\n");
        m_listenfd=CreateSocket();
        Bind_IPv4(port);
        Listen(5);
        LOG_INFO("等待客户端请求中...\n");
        //this->Poll();
        this->Epoll();

    }
private:
    int CreateSocket(){
        // 创建一个监听套接字
        int _socket = socket(PF_INET, SOCK_STREAM, 0);
        if(_socket == -1) {
            LOG_ERROR("创建socket出错：%s(errno:%d)\n",strerror(errno),errno);
        }
        else {
            LOG_DEBUG("创建socket成功\n");
        }
        return _socket;
    }
    void Bind_IPv4(int port){
        m_port=port;
        // 创建一个IPv4的socket地址
        struct sockaddr_in address;//socket地址
        bzero(&address, sizeof(address));//将该地址清空为0
        address.sin_family = AF_INET;//设置IPv4的地址族
        //inet_pton(AF_INET,ip,&address.sin_addr);//手动设置IP地址
        address.sin_addr.s_addr = htonl(INADDR_ANY);// 自动获取本机的IP地址并且设置
        address.sin_port = htons(m_port);//端口号
        //绑定socket地址
        int flag = 1;
        setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));//允许重用本地地址和端口
        int ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));//绑定socket地址
        if(ret == -1) {
            LOG_ERROR("绑定socket出错：%s(errno:%d)\n",strerror(errno),errno);
        }
        else {
            LOG_DEBUG("绑定socket成功\n");
        }
    }
    void Listen(int backlog){
        // 开启监听
        int ret = listen(m_listenfd, backlog);
        if(ret == -1) {
            LOG_ERROR("监听socket出错：%s(errno:%d)\n",strerror(errno),errno);
        }
        else {
            LOG_DEBUG("监听socket成功\n");
        }
    }
   
    void Epoll(){
        // 创建epoll模型
        int epollfd=epoll_create(5); 
        assert(epollfd!=-1); 
        // 注册epoll事件
        epoll_event events[MAX_EVENT_NUMBER]; 
        addfd(epollfd,m_listenfd,true);
        while (1)
        {
            int ret=epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1); //阻塞监听
            if(ret<0) {
                printf("epoll failure\n"); 
                break; 
            }
            //lt(events,ret,epollfd,m_listenfd);/*使用LT模式*/ 
            et(events,ret,epollfd,m_listenfd);/*使用ET模式*/ 
        }
        close(m_listenfd); 
    }
    /*将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核事件表中*/ 
    void addfd(int epollfd,int fd,bool enable_et) {
        epoll_event event; 
        event.data.fd=fd; //文件描述符
        event.events=EPOLLIN; //可读事件
        if(enable_et) {event.events|=EPOLLET; }//是否启用ET模式
        epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event); //注册epoll事件
        setnonblocking(fd); //设置为非阻塞模式
    }
    /*LT模式的工作流程*/ 
    void lt(epoll_event*events,int number,int epollfd,int listenfd) {
        char buf[BUFFER_SIZE]; 
        // 遍历events[number]事件数组
        for(int i=0;i<number;i++) {
            int sockfd=events[i].data.fd; 
            // 如果是服务端socket
            if(sockfd==listenfd) {
                struct sockaddr_in client_address; 
                socklen_t client_addrlength=sizeof(client_address); 
                int connfd=accept(listenfd,(struct sockaddr*)&client_address, &client_addrlength); 
                addfd(epollfd,connfd,false);//监听新的epoll事件，对connfd禁用ET模式
            }
            // 如果是可读事件
            else if(events[i].events &EPOLLIN){
                /*只要socket读缓存中还有未读出的数据，这段代码就被触发*/ 
                printf("event trigger once\n"); 
                memset(buf,'\0',BUFFER_SIZE); 
                int ret=recv(sockfd,buf,BUFFER_SIZE-1,0); //接受数据
                if(ret<=0) {
                    close(sockfd); 
                    continue; 
                }
                printf("get%d bytes of content:%s\n",ret,buf); 
            }
             // 如果是其他事件
            else {
                printf("something else happened\n"); 
            }
        }
    }
    /*ET模式的工作流程*/ 
    void et(epoll_event*events,int number,int epollfd,int listenfd) {
        char buf[BUFFER_SIZE]; 
        // 遍历events[number]事件数组
        for(int i=0;i<number;i++) {
            int sockfd=events[i].data.fd; 
            if(sockfd==listenfd) {
                struct sockaddr_in client_address; 
                socklen_t client_addrlength=sizeof(client_address); 
                int connfd=accept(listenfd,(struct sockaddr*)&client_address,& client_addrlength); 
                addfd(epollfd,connfd,true);
                /*对connfd开启ET模式*/ 
            }else if(events[i].events&EPOLLIN) {
                /*这段代码不会被重复触发，所以我们循环读取数据，以确保把socket读缓存中的所有 数据读出*/ 
                printf("event trigger once\n"); 
                while(1) {
                    memset(buf,'\0',BUFFER_SIZE); 
                    int ret=recv(sockfd,buf,BUFFER_SIZE-1,0); 
                    if(ret<0) {
                        /*对于非阻塞IO，下面的条件成立表示数据已经全部读取完毕。
                        此后，epoll就能再次触 发sockfd上的EPOLLIN事件，以驱动下一次读操作*/ 
                        if((errno==EAGAIN)||(errno==EWOULDBLOCK)) {
                            printf("read later\n");
                            break; 
                        }
                        close(sockfd); 
                        break; 
                    }else if(ret==0) {
                        close(sockfd); 
                    }else {
                        printf("get%d bytes of content:%s\n",ret,buf); 
                    }
                }
            }else {
                printf("something else happened\n"); 
            }
        }
    }
private:
    int m_port;//端口号
    int  m_listenfd;//服务端server
};

int main(int argc, char * argv[]){
    Server_Epoll* server = new Server_Epoll();
    server->run(DEFAULT_PORT);
    return 0;
}

