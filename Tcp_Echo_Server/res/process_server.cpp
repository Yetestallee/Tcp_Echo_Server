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

// 回收僵尸进程
void handle_sigchld(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// 日志函数（多进程不用，但保留兼容）
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
void write_log(const char* client_ip, int client_port, const char* event) {
    pthread_mutex_lock(&log_mutex);
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    FILE* fp = fopen(LOG_FILE, "a");
    if (fp) {
        fprintf(fp, "[%s] %s:%d - %s\n", time_str, client_ip, client_port, event);
        fclose(fp);
    }
    pthread_mutex_unlock(&log_mutex);
}

// ===================== 子进程处理客户端 =====================
void handle_client_process(int client_fd, struct sockaddr_in client_addr, int server_fd) {
    close(server_fd); // 关闭监听套接字

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, INET_ADDRSTRLEN);
    int port = ntohs(client_addr.sin_port);

    char buf[BUF_SIZE];
    while (1) {
        ssize_t n = recv(client_fd, buf, BUF_SIZE, 0);
        if (n <= 0) break;

        send(client_fd, buf, n, 0);
        memset(buf, 0, n);
    }

    close(client_fd);
    exit(0);
}

// ===================== 主函数 =====================
int main() {
    signal(SIGCHLD, handle_sigchld);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    optimize_socket(server_fd);

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, MAX_CONN);

    printf("多进程服务器已启动 :%d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);

        pid_t pid = fork();
        if (pid == 0) {
            handle_client_process(client_fd, client_addr, server_fd);
        } else {
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}
