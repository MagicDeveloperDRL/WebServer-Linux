/*
 * @Author: WJ
 * @Date: 2022-04-17 22:17:35
 * @Description: Linux下的log日志工具
 * @LastEditTime: 2022-05-13 21:44:23
 * @FilePath: \C++\code\log\log.h
 */

# ifndef WJ_LOGGER_H_
# define  WJ_LOGGER_H_

#include<iostream>
# include <fstream>
# include <string>
# include <time.h>
# include <stdio.h>
# include <stdlib.h>
#include<stdarg.h>
using namespace std;


class Logger{
public:
     //枚举类型
    enum log_level{debug,info,warning,error};//日志等级
    enum log_target{file,terminal,file_and_terminal};//日志输出目标
private:
    //构造函数被保护，不能被外界访问
    Logger();//默认构造函数
    Logger(log_target target,log_level level,string path);
    ofstream outfile;//将日志输出到文件的流对象

    log_target target;//日志输出目标
    string path; //日志文件路径
    log_level level;// 日志等级
    //获取当前时间，并格式化表示
    string currTime(){
        char tmp[64];
        time_t ptime;
        time(&ptime);
        strftime(tmp,sizeof(tmp),"%Y-%M-%d %H:%M:%S",localtime(&ptime));
        return tmp;
    }
        
       
public:
    //设置一个公有的静态成员函数，用来选择作为外界唯一的创建接口
    static Logger* Instance();
    void set_target(log_target _target,string _path="./log.txt"){
        this->target=_target;
        this->path=_path;
        if(target!=log_target::terminal){
            this->outfile.open(path,ios::out|ios::app);//打开输出文件
       }
    }
    void output(log_level act_level,const char* format,...);//输出行为
};

//是否输出相关等级信息
#define DEBUG_OUT 1
#define INFO_OUT 1
#define WARNING_OUT 1
#define ERROR_OUT   1
// 输出宏工具
#define LOG_INFO(format,...) {Logger::Instance()->output(Logger::log_level::info,format,##__VA_ARGS__);}
#define LOG_DEBUG(format,...) {Logger::Instance()->output(Logger::log_level::debug,format,##__VA_ARGS__);}
#define LOG_WARN(format,...) {Logger::Instance()->output(Logger::log_level::warning,format,##__VA_ARGS__);}
#define LOG_ERROR(format,...) {Logger::Instance()->output(Logger::log_level::error,format,##__VA_ARGS__);}

# endif //WJ_LOGGER_H_