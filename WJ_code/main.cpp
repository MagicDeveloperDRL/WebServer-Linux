/*
 * @Author: WJ
 * @Date: 2022-03-21 20:43:14
 * @Description: 服务端程序启动入口
 * @LastEditTime: 2022-05-14 20:24:11
 * @FilePath: \WebServer\main.cpp
 */
#include"log/log.h"
#include"server/webserver.h"
//#include"../code/log/log.h"
#include<iostream>
using namespace std;

int main(){
    printf("输出\n");
  //  LOG_INFO("Info %d,%d",a,b);
    //LOG_DEBUG("debug");
   // LOG_WARN("warning");
   // LOG_ERROR("error");

    WebServer *server=new WebServer(
        1316, 3, 60000, false,             /* 端口 ET模式 timeoutMs 优雅退出  */
        3306, "debian-sys-maint", "FMLmRNlo5VPsJDUD", "webserver", /* 数据库端口，用户名，密码，数据库名称 */
        12, 6,   /* 连接池数量 线程池数量 */
        true, 1, 1024          /* 输出日志开关 日志等级 日志异步队列容量 */
    );
     // 启动Web服务器
    server->Run();
    return 0;
}