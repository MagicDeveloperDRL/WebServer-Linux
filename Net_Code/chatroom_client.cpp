/*
 * @Author: MRL Liu
 * @Date: 2022-03-29 14:36:42
 * @Description: Linux的基于Socket的聊天室客户端
 * @LastEditTime: 2022-04-02 17:34:03
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
#include<poll.h>//系统调用
#include"log/logger.h"

#define BUFFER_SIZE 64
#define DEFAULT_PORT 8000

void Client(const char*ip,int port){
    LOG_INFO("客户端程序启动...\n");
     // 创建一个IPv4的socket地址
    struct sockaddr_in server_address; 
    bzero(&server_address,sizeof(server_address)); 
    server_address.sin_family=AF_INET; 
    inet_pton(AF_INET,ip,&server_address.sin_addr); 
    server_address.sin_port=htons(port);
    // 创建一个监听套接字
    int sockfd=socket(PF_INET,SOCK_STREAM,0); 
    if(sockfd == -1) {
        LOG_ERROR("创建socket出错：%s(errno:%d)\n",strerror(errno),errno);
    }
    else {
        LOG_DEBUG("创建socket成功\n");
    }
    // 监听套接字绑定socket地址
    int ret=-1;
    ret=connect(sockfd,(struct sockaddr*)& server_address,sizeof(server_address));
    if(ret == -1) {
        LOG_ERROR("连接聊天室出错：%s(errno:%d)\n",strerror(errno),errno);
    }
    else {
        LOG_DEBUG("连接聊天室成功\n");
        pollfd fds[2];
        fds[0].fd=0;
        fds[0].events=POLLIN;
        fds[0].revents=0;
        fds[1].fd=sockfd;
        fds[1].events=POLLIN|POLLRDHUP;
        fds[1].revents=0;
        char read_buff[BUFFER_SIZE];
        int pipefd[2];
        int ret=pipe(pipefd);
        assert(ret!=-1);
        while(1){
            ret=poll(fds,2,-1);
            if(ret<0){
                LOG_ERROR("poll出错\n");
                break;
            }
            if(fds[1].revents&POLLRDHUP){
                LOG_INFO("服务端断开连接\n");
                break;
            }
            else if(fds[1].revents&POLLIN){
                memset(read_buff,'\0',BUFFER_SIZE);
                recv(fds[1].fd,read_buff,sizeof(read_buff)-1,0); 
                LOG_INFO("接收到服务端消息: %s\n",read_buff);
            }
            if(fds[0].revents&POLLIN){
                ret=splice(0,NULL,pipefd[1],NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
                ret=splice(pipefd[0],NULL,sockfd,NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
                LOG_DEBUG("向服务端发送成功\n");
            }
        }
    }
    // 关闭socket
    close(sockfd); 
}

int main(int argc, char * argv[]){
    if(argc<=2){
        Client("127.0.0.1",DEFAULT_PORT);
    }
    else{
        const char*ip=argv[1];
        int port=atoi(argv[2]);
        Client(ip,port);
    }
    return 0;
}

