#include "common.h"

// 套接字优化实现
int optimize_socket(int fd) {
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt SO_REUSEADDR");
        return -1;
    }

    int rcv_buf = 8 * 1024;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcv_buf, sizeof(rcv_buf)) == -1) {
        perror("setsockopt SO_RCVBUF");
        return -1;
    }

    int snd_buf = 8 * 1024;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &snd_buf, sizeof(snd_buf)) == -1) {
        perror("setsockopt SO_SNDBUF");
        return -1;
    }

    int keepalive = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) == -1) {
        perror("setsockopt SO_KEEPALIVE");
        return -1;
    }
    return 0;
}

// 日志互斥锁（全局，保证多线程写日志不混乱）
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// 日志写入函数
void write_log(const char* client_ip, int client_port, const char* event) {
    pthread_mutex_lock(&log_mutex);

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    FILE* fp = fopen(LOG_FILE, "a");
    if (fp != NULL) {
        fprintf(fp, "[%s] 客户端 %s:%d - %s\n", time_str, client_ip, client_port, event);
        fclose(fp);
    }

    pthread_mutex_unlock(&log_mutex);
}

// 线程函数参数结构体，传递客户端套接字和地址信息
typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
} ThreadData;

/**
 * @brief 线程函数，处理客户端回声逻辑
 * @param arg 线程参数（ThreadData结构体指针）
 * @return 线程退出返回值（NULL）
 * @details 线程负责接收客户端数据并回显，记录连接/断开日志，直到客户端断开
 */
void* handle_client_process(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int client_fd = data->client_fd;
    struct sockaddr_in client_addr = data->client_addr;
    free(data);  // 释放主线程分配的内存

    // 设置线程分离，无需主线程 join
    pthread_detach(pthread_self());

    // 解析客户端 IP 和端口
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client_addr.sin_port);

    // 记录连接日志
    write_log(client_ip, client_port, "已连接");
    printf("线程[%lu] 处理客户端: %s:%d\n", pthread_self(), client_ip, client_port);

    char buffer[BUF_SIZE] = {0};

    while (true) {
        // 阻塞接收客户端数据
        ssize_t recv_len = recv(client_fd, buffer, BUF_SIZE, 0);

        if (recv_len <= 0) {
            if (recv_len == 0) {
                printf("线程[%lu] 客户端断开连接: %s:%d\n", pthread_self(), client_ip, client_port);
                write_log(client_ip, client_port, "正常断开");
            } else {
                perror("recv 失败");
                write_log(client_ip, client_port, "接收数据异常");
            }
            break;
        }

        // 打印收到的数据
        printf("线程[%lu] 接收 [%s:%d]: %s", pthread_self(), client_ip, client_port, buffer);

        // 回显给客户端
        send(client_fd, buffer, recv_len, 0);

        // 清空缓冲区
        memset(buffer, 0, sizeof(buffer));
    }

    // 关闭客户端套接字
    close(client_fd);
    printf("线程[%lu] 处理完毕，退出\n", pthread_self());

    return NULL;
}

// 主函数：创建监听套接字 + 接收连接 + 创建线程
int main() {
    // 1. 创建 TCP 套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket 创建失败");
        exit(EXIT_FAILURE);
    }

    // 2. 优化套接字
    if (optimize_socket(server_fd) == -1) {
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. 配置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // 4. 绑定
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind 失败");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 5. 监听
    if (listen(server_fd, MAX_CONN) == -1) {
        perror("listen 失败");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("===== 多线程回声服务器已启动 =====\n");
    printf("监听端口: %d\n", PORT);
    printf("日志文件: %s\n", LOG_FILE);
    printf("=================================\n");

    // 6. 循环接受客户端连接
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // 阻塞等待客户端连接
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept 失败");
            continue;
        }

        // 分配线程参数内存
        ThreadData* data = (ThreadData*)malloc(sizeof(ThreadData));
        if (data == NULL) {
            perror("malloc 失败");
            close(client_fd);
            continue;
        }

        data->client_fd = client_fd;
        data->client_addr = client_addr;

        // 创建线程
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client_process, (void*)data) != 0) {
            perror("线程创建失败");
            free(data);
            close(client_fd);
            continue;
        }

        printf("主线程：已创建线程 [%lu] 处理新客户端\n", tid);
    }

    // 理论上不会运行到这里
    close(server_fd);
    pthread_mutex_destroy(&log_mutex);
    return 0;
}
