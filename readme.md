# Web简易服务器项目
该项目是作者在学习Linux服务端编程时整理的项目，Web简易服务器是一款用C++实现的基于Linux的**轻量级高性能Web服务器**，经过web bench工具的压力测试，可以实现上万的**QPS** (Query Per Second，每秒查询率)。

## 技术栈

* 基于**多进程网络通信**和**匿名管道技术**的**Web bench**测试工具；
* 基于**互斥锁**和**条件变量**等多线程同步技术实现的**线程池模块**，实现将多个任务同时分配给多个线程；
* 基于**Socket编程**和**IO复用技术**Epoll实现的**Reactor高并发模型**，实现同时监听多个客户端事件；
* 基于**最小堆结构**的**定时器模块**，实现非活动连接的超时管理；
* 基于**正则表达式**和**状态机**的**HTTP/1.1**解析和响应模块，实现处理**Web静态资源**的请求；
* 支持**分散读**和**集中写**的**缓冲区模块**，实现缓存数据的合理管理，提高**文件数据拷贝**效率；
* 基于**RAII机制**实现的**MySQL数据库连接池模块**，减少数据库连接建立与关闭的开销；
* 基于**单例模式**和**阻塞队列**的异步日志系统，实现以文件形式按日期记录服务器运行信息；

## 环境要求
* Linux操作系统
* C++14编程语言
* MySql数据库
* MakeFIle项目编译工具

## 目录树
```tex
.
├── code           源代码
│   ├── buffer     缓存区模块    
│   ├── http       HTTP模块
│   ├── log        简易日志模块
│   ├── log1       异步日志模块
│   ├── pool       线程池和MySQL连接池
│   ├── server     socket模块、Epoll模块和主程序
│   ├── timer      定时器模块
│   └── main.cpp   入口函数
├── web            Web静态资源
│   ├── index.html 网站首页
│   ├── images     图片资源
│   ├── video      视频资源
│   ├── js         脚本资源
│   └── css        样式资源
├── bin            可执行文件
│   └── WebServer_v1.0
├── log            日志文件
├── webbench-1.5   第三方压力测试工具
├── build          编译指令 
│   └── Makefile
├── Makefile       启动指令  
├── LICENSE
└── readme.md
```


## 项目启动
需要先配置好对应的数据库
```bash
// 建立yourdb库
create database yourdb;

// 创建user表
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, password) VALUES('name', 'password');
```

```bash
make
./bin/WebServer_v1.0
```

## 压力测试
```bash
./webbench-1.5/webbench -c 100 -t 10 http://ip:port/
./webbench-1.5/webbench -c 1000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 5000 -t 10 http://ip:port/
./webbench-1.5/webbench -c 10000 -t 10 http://ip:port/
```
* 测试环境: Ubuntu:19.10 cpu:i5-8400 内存:8G 
* QPS 10000+

## 参考资料
- [TinyWebServer: Linux下C++轻量级Web服务器 ](https://github.com/qinguoyi/TinyWebServer)
- 《Linux高性能服务器编程》，游双著.
