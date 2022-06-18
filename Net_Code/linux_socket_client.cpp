/*
 * @Author: MRL Liu
 * @Date: 2022-03-29 14:36:42
 * @Description: Linux的Socket客户端
 * @LastEditTime: 2022-04-02 16:26:28
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
        LOG_ERROR("连接服务端出错：%s(errno:%d)\n",strerror(errno),errno);
    }
    else {
        LOG_DEBUG("连接服务端成功\n");
        const char*oob_data="abc"; 
        const char*normal_data="123"; 
        send(sockfd,normal_data,strlen(normal_data),0); //发送正常数据
        send(sockfd,oob_data,strlen(oob_data),MSG_OOB); //发送紧急数据
        send(sockfd,normal_data,strlen(normal_data),0);//发送正常数据
        LOG_DEBUG("向服务端发送成功\n");
        char buff[4096];
        int n =recv(sockfd,buff,sizeof(buff),0); 
        buff[n] = '\0';
        LOG_INFO("接收到服务端消息: %s\n",buff);
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

