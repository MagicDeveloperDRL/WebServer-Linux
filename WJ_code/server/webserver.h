/*
 * @Author: your name
 * @Date: 2022-05-13 21:14:58
 * @LastEditTime: 2022-05-14 20:22:17
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /WebServer/WJ_code/sever/webserver.h
 */

#ifndef WJ_WEB_SERVER_H_
#define WJ_WEB_SERVER_H_

#include<iostream>
#include<memory>
#include <unistd.h>     
#include <arpa/inet.h>  
#include <unordered_map>
//#include <fcntl.h>       // fcntl()

#include<string>
#include<netdb.h>

#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include"socket.h"

#include "../log/log.h" // 日志系统


// Web服务器类
class WebServer{
public:
    WebServer(
                // 端口号，epoll触发模式
                int port, int trigMode, int timeoutMS, bool OptLinger,
                // SQL端口号，SQL用户名，SQL密码， 数据库名称，连接池数量
                int sqlPort, const char* sqlUser, const  char* sqlPwd,const char* dbName, int connPoolNum, 
                int threadNum,
                // 日志系统：
                bool openLog, int logLevel, int logQueSize
    );//构造函数
    ~WebServer();// 析构函数
public:
    void Run();//启动
private:
    void InitListenSocket(int port,bool OptLinger);

    char* srcDir_;// 当前网页资源所在目录
    bool isClose_;// 是否关闭服务器
    int timeoutMS_;  /* 毫秒MS */
    uint32_t listenEvent_;// 监听socket触发模式
    uint32_t connEvent_;// 连接socket触发模式
    // 基础服务模块
    std::unique_ptr<ListenSocket> m_pSocket;//套接字模块指针
    
    
};
#endif // WJ_WEB_SERVER_H_