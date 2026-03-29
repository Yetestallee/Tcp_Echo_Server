#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <ctime>
#include <fcntl.h>

#define PORT 8888
#define BUF_SIZE 1024
#define MAX_CONN 5
#define LOG_FILE "server.log"

// 【仅声明】函数实现放到cpp文件中
int optimize_socket(int fd);
void handle_sigchld(int sig);
void write_log(const char* client_ip, int client_port, const char* event);

// 线程安全日志锁（声明）
extern pthread_mutex_t log_mutex;

#endif
