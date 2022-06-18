/*
 * @Author: WJ
 * @Date: 2022-04-17 22:17:35
 * @Description: Linux下的log日志工具
 * @LastEditTime: 2022-05-13 18:44:16
 * @FilePath: \C++\code\log\log.cpp
 */

#include"log.h"

//构造函数
Logger::Logger(){
    this->target=terminal;
    this->level=debug;
} 

Logger::Logger(log_target target,log_level level,string path){
    this->target=target;
    this->path=path;
    this->level=level;
    if(target!=terminal){
        this->outfile.open(path,ios::out|ios::app);//打开输出文件
    }
}

void Logger::output( log_level act_level,const char *format,...){
    // 分析输出等级
    string prefix;
    switch(act_level){
        case log_level::debug:
            #ifndef DEBUG_OUT
                return;
            #endif
            prefix = "[DEBUG]   ";
            break;
        case log_level::info:
            #ifndef INFO_OUT
                return;
            #endif
            prefix = "[INFO]    ";
            break;
        case log_level::warning:
            #ifndef WARNING_OUT
                return;
            #endif
            prefix = "[WARNING]    ";
            break;
        case log_level::error:
            #ifndef ERROR_OUT
                return;
            #endif
            prefix = "[ERROR]    ";
            break;
        default:
            prefix = "";
    }
    //prefix += __FILE__;
    prefix += " "+ currTime() + " : ";
    string output_content = prefix + format + "\n";
    if(this->level <= act_level && this->target != file){
        // 当前等级设定的等级才会显示在终端，且不能是只文件模式
        cout<<(prefix);
        va_list args;// 定义一个va_list类型的变量，用来存储单个参数
        va_start(args,format);// 使args指向可变参数的第一个参数
        vprintf(format,args);
        va_end(args);//结束可变参数的获取
        cout<<endl;
    }
    if(this->target != Logger:: log_target::terminal){
        outfile << output_content;
    }
}


// 静态成员函数
Logger* Logger::Instance(){
    static  Logger _instance;//静态局部变量
    return &_instance;
}