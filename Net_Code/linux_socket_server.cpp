/*
 * @Author: MRL Liu
 * @Date: 2022-03-29 14:36:42
 * @Description: Linux的Socket服务端
 * @LastEditTime: 2022-04-02 17:05:38
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
#include"log/logger.h"

#define MAXLINE 4096
#define DEFAULT_PORT 8000



void Server(int port){
    LOG_INFO("服务端程序启动...\n");
    // 创建一个IPv4的socket地址
    struct sockaddr_in address;//socket地址
    bzero(&address, sizeof(address));//将该地址清空为0
    address.sin_family = AF_INET;//设置IPv4的地址族
    //inet_pton(AF_INET,ip,&address.sin_addr);//手动设置IP地址
    address.sin_addr.s_addr = htonl(INADDR_ANY);// 自动获取本机的IP地址并且设置
    address.sin_port = htons(port);//端口号
    // 创建一个监听套接字
    int m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(m_listenfd == -1) {
        LOG_ERROR("创建socket出错：%s(errno:%d)\n",strerror(errno),errno);
    }
    else {
        LOG_DEBUG("创建socket成功\n");
    }
    // 监听套接字绑定socket地址
    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));//允许重用本地地址和端口
    int ret = bind(m_listenfd, (struct sockaddr *)&address, sizeof(address));//绑定socket地址
    if(ret == -1) {
        LOG_ERROR("绑定socket出错：%s(errno:%d)\n",strerror(errno),errno);
    }
    else {
        LOG_DEBUG("绑定socket成功\n");
    }
    // 开启监听
    ret = listen(m_listenfd, 5);
    if(ret == -1) {
        LOG_ERROR("监听socket出错：%s(errno:%d)\n",strerror(errno),errno);
    }
    else {
        LOG_DEBUG("监听socket成功\n");
    }
    // 开启监听循环
    int connfd=-1;
    char buff[4096];
    fd_set read_fds;//可读文件描述符集合
    fd_set exception_fds;//异常文件描述符集合
    FD_ZERO(&read_fds);
    FD_ZERO(&exception_fds);
     LOG_INFO("等待客户端请求中...\n");
    while(1){
        // 阻塞等待接受客户端的连接
        struct sockaddr_in client;
        socklen_t client_addrlen=sizeof(client);
        connfd=accept(m_listenfd,(struct sockaddr*)&client,&client_addrlen);
        //connfd=accept(m_listenfd,(struct sockaddr*)NULL,NULL);
        // 如果连接失败
        if(connfd==-1){
            LOG_ERROR("连接socket出错: %s(errno: %d)\n",strerror(errno),errno);
            continue;
        }
        // 如果连接成功
        else{
            // 输出客户端的IP地址和端口号
            char remote[INET_ADDRSTRLEN]; 
            LOG_INFO("连接到客户端,ip：%s and port：%d\n",inet_ntop(AF_INET,&client.sin_addr,remote,INET_ADDRSTRLEN),ntohs(client.sin_port));
        }
        {
            memset(buff,'\0',sizeof(buff));
            // 从客户端接受数据
            FD_SET(connfd,&read_fds);
            FD_SET(connfd,&exception_fds);
            ret=select(connfd+1,&read_fds,NULL,&exception_fds,NULL);
            if(ret<0) break;
            // 对于可读事件
            if(FD_ISSET(connfd,&read_fds)){
                ret = recv(connfd,buff,sizeof(buff)-1,0);//从connfd中接收最大长度为ΪMAXLINE的数据到buff中，返回接收到的长度
                if(ret<=0) break;
                LOG_INFO("接收到客户端%d字节的正常数据: %s\n",ret,buff);
            }
            // 对于异常事件
            else if(FD_ISSET(connfd,&exception_fds)){
                ret = recv(connfd,buff,sizeof(buff)-1,MSG_OOB);//从connfd中接收最大长度为ΪMAXLINE的数据到buff中，返回接收到的长度
                if(ret<=0) break;
                LOG_INFO("接收到客户端%d字节的带外数据: %s\n",ret,buff);
            }
        }
        if(!fork()){
            if(send(connfd,"你好，服务端欢迎你\n",26,0)==-1){
                LOG_ERROR("给客户端发送消息出错: %s(errno: %d)\n",strerror(errno),errno);
            }
            close(connfd);
            exit(0);
        }
        // 关闭客户端
        close(connfd);
    }
    // 关闭socket
    close(m_listenfd);
}



int main(int argc, char * argv[]){
    Server(DEFAULT_PORT);
    return 0;
}

