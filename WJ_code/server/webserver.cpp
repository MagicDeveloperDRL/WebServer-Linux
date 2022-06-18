/*
 * @Author: your name
 * @Date: 2022-05-13 21:14:40
 * @LastEditTime: 2022-05-14 20:23:54
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /WebServer/WJ_code/sever/websever.cpp
 */

#include"webserver.h"
#include<iostream>
using namespace std;

// 构造函数
WebServer::WebServer(
            // 端口号，epoll触发模式,最大等待时间，是否优雅断开socket
            int port, int trigMode, int timeoutMS, bool OptLinger,
            // SQL端口号，SQL用户名，SQL密码， 数据库名称，连接池数量
            int sqlPort, const char* sqlUser, const  char* sqlPwd,const char* dbName, int connPoolNum, 
            int threadNum,
            // 是否开启日志，日志，日志队列的容量
            bool openLog, int logLevel, int logQueSize
    ):isClose_(false),timeoutMS_(timeoutMS),
    m_pSocket(new ListenSocket()) {
    
    this->InitListenSocket(port,OptLinger); // 初始化监听socket
   
    //  初始化MySQL数据库连接池
   
    // 初始化日志系统
    if(openLog) {
        //Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        Logger::Instance()->set_target(Logger::log_target::file_and_terminal,"./log.txt");
        if(isClose_) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port, OptLinger? "true":"false");
            
            LOG_INFO("LogSys level: %d", logLevel);
            
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}


// 启动
void WebServer::Run(){
   
    if(!isClose_) { 
        LOG_INFO("========== Server start ==========");
        cout<<"服务器程序运行.."<<endl;
     }
        
    
}

void WebServer::InitListenSocket(int port,bool OptLinger){
    // 初始化监听socket
    if(!m_pSocket->Init(port,OptLinger)){
        isClose_ = true;
    }
}

















