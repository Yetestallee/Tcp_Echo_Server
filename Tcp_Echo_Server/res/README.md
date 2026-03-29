该项目基于 Linux 下的 Socket 编程，实现了多线程、多进程两种版本的 TCP 回声服务器，以及配套的客户端程序。服务器接收客户端发送的数据并原样回显，包含套接字优化、日志记录、僵尸进程回收等工业级特性。
一、项目特性
✅ 支持多线程 / 多进程并发处理客户端连接
✅ 套接字优化（端口复用、收发缓冲区调整、保活机制）
✅ 完善的日志记录（客户端连接 / 断开 / 异常日志）
✅ 僵尸进程自动回收（多进程版本）
✅ 客户端支持 exit 指令安全退出
✅ 跨函数 / 文件的公共常量与函数封装（common.h）
二、环境要求
表格
环境 / 工具	要求
操作系统	Linux（Ubuntu/CentOS 等，依赖 POSIX 接口）
编译器	GCC（支持 C++11 及以上）
依赖	标准库（pthread、socket、netinet 等）
三、文件结构
plaintext
.
├── common.h          # 公共常量、函数声明（端口、缓冲区、日志路径等）
├── client.cpp        # TCP 客户端实现（连接服务器、收发数据）
├── thread_server.cpp # 多线程版本服务器（并发处理客户端）
├── process_server.cpp# 多进程版本服务器（fork 子进程处理客户端）
├── server.log        # 日志文件（自动生成，记录客户端连接/断开日志）
└── README.md         # 项目说明文档
四、编译步骤
方式 1：手动编译（单文件）

# 编译客户端
g++ client.cpp -o client -lpthread

# 编译多线程服务器
g++ thread_server.cpp -o thread_server -lpthread

# 编译多进程服务器
g++ process_server.cpp -o process_server -lpthread
方式 2：Makefile（推荐，简化编译）
创建 Makefile 文件，内容如下：
makefile
CC = g++
CFLAGS = -Wall -Wextra -lpthread
TARGETS = client thread_server process_server

all: $(TARGETS)

client: client.cpp common.h
	$(CC) $< -o $@ $(CFLAGS)

thread_server: thread_server.cpp common.h
	$(CC) $< -o $@ $(CFLAGS)

process_server: process_server.cpp common.h
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -f $(TARGETS) server.log
执行编译：
make
清理编译产物：
make clean
五、运行步骤
1. 启动服务器
方式 A：多线程服务器
./thread_server
输出示例：
plaintext
===== 多线程回声服务器已启动 =====
监听端口: 8888
日志文件: server.log
=================================
方式 B：多进程服务器
./process_server
输出示例：
plaintext
多进程服务器已启动 :8888
2. 启动客户端
# 格式：./client <服务器IP地址>
./client 127.0.0.1  # 本地测试用 127.0.0.1，远程服务器替换为实际IP
输出示例：
plaintext
成功连接到服务器 127.0.0.1:8888
请输入要发送的数据：
3. 功能测试
在客户端输入任意文本，服务器会原样回显：
plaintext
请输入要发送的数据：Hello Server!
服务器返回的数据：Hello Server!

请输入要发送的数据：exit  # 输入 exit 退出客户端
六、核心功能说明
表格
模块	说明
套接字优化	optimize_socket 函数实现：
- SO_REUSEADDR：端口复用，避免服务器重启后端口占用
- SO_RCVBUF/SO_SNDBUF：调整收发缓冲区大小（8KB）
- SO_KEEPALIVE：开启 TCP 保活，检测无效连接
日志记录	write_log 函数：
- 线程安全（互斥锁），记录客户端 IP / 端口、事件（连接 / 断开 / 异常）
- 日志文件：server.log，格式 [时间] 客户端 IP:端口 - 事件
多线程处理	主线程监听连接，每个客户端连接创建独立线程处理，线程分离（无需主线程 join）
多进程处理	主线程监听连接，fork 子进程处理客户端，信号处理函数回收僵尸进程
客户端退出	输入 exit 触发退出逻辑，关闭套接字并释放资源
七、注意事项
端口占用：默认端口为 8888，若端口被占用，可修改 common.h 中的 PORT 常量（需重新编译）。
日志文件：服务器运行时自动生成 server.log，记录客户端连接 / 断开日志，可直接查看：
cat server.log
权限问题：若编译 / 运行时报权限错误，执行 chmod +x client thread_server process_server 赋予可执行权限。
远程连接：若客户端需连接远程服务器，需确保服务器防火墙开放 8888 端口，且服务器绑定 0.0.0.0（代码中已设置 INADDR_ANY）。
异常处理：代码包含基础的错误处理（如 socket 创建失败、connect 失败、recv/send 异常），可根据需求扩展。
八、扩展建议
增加客户端超时机制（设置 SO_RCVTIMEO/SO_SNDTIMEO）。
支持二进制数据传输（当前仅支持文本）。
增加服务器配置文件（端口、缓冲区大小等从配置文件读取）。
实现客户端 / 服务器的断线重连机制。
增加日志轮转（避免 server.log 过大）。# TCP 回声服务器 / 客户端（C++ 实现）
