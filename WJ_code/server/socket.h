/*
 * @Author: your name
 * @Date: 2022-05-13 21:14:17
 * @LastEditTime: 2022-05-14 20:25:20
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /WebServer/WJ_code/sever/socket.h
 */

#ifndef WJ_SOCKET_H_
#define WJ_SOCKET_H_
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include <fcntl.h>  // 设置非阻塞
#include<errno.h>


#include <iostream>
#include <unistd.h>  // write
#include<assert.h>//assert
#include<string.h>//strlen
#include "../log/log.h" // 日志系统
 class ListenSocket{
public: 
    ListenSocket(){
        m_port=-1;
        m_listenfd=-1;
    }
    ~ListenSocket()=default;
    bool Init(int port,bool openLinger);
    int Accept(struct sockaddr_in* addr_);
    void Close(){close(m_listenfd);};
    const char * GetIP();
    int GetPort(){return m_port;}
    int GetListenFd(){return m_listenfd;}
    int SetFdNonblock(int fd);
private:
    bool CreateSocket();
    bool Bind_IPv4(int port);
    bool SetLinger(bool openLinger_);//设置TCP的断开方式
    bool Listen(int backlog);
private:
    int m_port;//端口号
    int  m_listenfd;//服务端server
};

#endif //WJ_SOCKET_H_