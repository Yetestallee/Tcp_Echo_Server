C++ TCP Echo Server

一个基于 C++ 实现的 TCP 回声服务器系统，支持多线程与多进程两种高并发处理模型。

📖 简介

本项目是一个轻量级的 TCP 回声服务器实现，用于演示 Linux 下 C++ 网络编程的核心概念。它支持两种并发处理模式：



- 多线程模型（thread_server）：每个客户端连接由独立线程处理



- 多进程模型（process_server）：每个客户端连接由独立子进程处理  

配套提供客户端程序，可用于连接测试与通信验证。系统具备基础网络通信能力、连接管理与运行日志记录功能，适用于教学演示与技术学习。

🚀 快速开始

只需三步即可在本地运行服务并进行测试：

1. 克隆项目

git clone https://github.com/example/tcp-echo-server.git

cd tcp-echo-server


2. 编译所有组件

make all

3. 启动服务器并连接测试

打开两个终端窗口：

终端 A - 启动多线程服务器

./thread_server

终端 B - 运行客户端连接

./client 127.0.0.1

输入任意消息后回车，服务器将原样返回；输入 exit 可断开连接。

✨ 功能特性

• ⚡双模型支持：灵活选择多线程或多进程并发方案，适应不同场景需求

• 🔒线程安全日志：使用互斥锁保护日志写入，防止多线程竞争导致数据错乱

• 🧼资源自动清理：多线程中调用pthread_detach自动释放资源，多进程中通过waitpid回收僵尸进程

• 🛠️套接字优化：启用SO_REUSEADDR和SO_KEEPALIVE提升连接健壮性与可靠性

• 📦模块化设计：common.h统一管理公共配置与工具函数，便于维护与扩展

🧩 模块说明


| 模块 | 类型 | 功能描述 |
| --- | --- | --- |
| thread_server | 可执行文件 | 多线程服务器，主线程监听连接请求，为每个客户 |


|  |  | 端创建独立工作线程处理通信 |
| --- | --- | --- |
| process_server | 可执行文件 | 多进程服务器，父进程仅负责监听与接受连接，通过 fork() 创建子进程接管会话 |
| client | 可执行文件 | 支持交互式发送消息的 TCP 客户端，可指定服务器 IP 地址进行连接测试 |
| common.h | 头文件 | 定义通用宏常量、包含系统头文件、声明共享函数与全局互斥锁实例 |




🛠️ 构建说明

项目包含简易 Makefile，支持一键编译：

# 编译全部组件

make all



# 单独编译某个模块

make thread_server

make process_server

make client



# 清理生成文件

make clean


若无 Makefile，也可手动编译（需链接 pthread 库）：

g++ -o thread_server thread_server.cpp common.cpp -lpthread

g++ -o process_server process_server.cpp common.cpp

g++ -o client client.cpp common.cpp

构建环境要求：POSIX 兼容操作系统（如 Linux 或 macOS），具备 g++ 编译器与 pthread 支持。

🤝 使用示例

启动多进程服务器

./process_server

客户端连接远程服务器

./client 192.168.1.100

查看运行日志

连接事件将记录至本地日志文件：

cat server.log

🔄 并发模型对比


| 特性 | 多线程模型 | 多进程模型 |
| --- | --- | --- |
| 创建方式 | pthread_create() | fork() |
| 主控流程 | 主线程负责 accept 连接 | 父进程仅负责 accept 连接 |
| 工作单元 | 子线程处理客户端数据收发 | 子进程接管与客户端的完整通信 |
| 资源回收 | pthread_detach() 实现线程自动释放 | 注册 SIGCHLD 信号处理器，调用 waitpid() 回收僵 |


|  |  | 尸进程 |
| --- | --- | --- |
| 内存隔离 | 线程间共享地址空间，需同步访问公共资源 | 进程间内存独立，避免相互干扰 |
| 日志安全 | 使用 log_mutex 保证写操作原子性 | 函数内部加锁保障多进程环境下的日志安全 |




💡 技术细节

• 端口配置：默认监听8080端口，可通过修改common.h中的PORT宏进行调整

• 缓冲区大小：BUF_SIZE=1024字节，定义单次读取最大数据长度

• 最大连接数：MAX_CONN=10，适用于教学演示场景

• 日志文件：输出至LOG_FILE="server.log"，记录连接建立、断开及异常事件

所有参数均可通过编辑 common.h 文件进行自定义配置，重新编译后生效。

❓ 常见问题

Q: 启动时报错 "Address already in use"？

A: 启用 SO_REUSEADDR 选项后通常可解决此问题。等待几秒或更换端口即可恢复。

Q: 如何关闭服务器？

A: 在服务器终端按下 Ctrl+C 即可终止进程。

Q: 是否支持 Windows？



A: 当前仅支持类 Unix 系统（Linux/macOS），依赖 POSIX 系统调用和 pthread 库。
