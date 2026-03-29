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


int main(int argc,char* argv[]){
    //检查命令行参数，确保用户提供了服务器IP地址
    if(argc!=2){
        fprintf(stderr,"Usage: %s <server_ip>\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    //创建tcp套接字
    int client_fd = socket(AF_INET,SOCK_STREAM,0);
    //检查是否创建成功
    if(client_fd==-1){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
     // 2. 优化套接字（可选，客户端也可优化）
    if (optimize_socket(client_fd) == -1) {
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    //配置服务器
    struct sockaddr_in server_addr;
    //设置为IPV4地址
    server_addr.sin_family=AF_INET;
    //清空结构体，避免垃圾数据
    memset(&server_addr, 0, sizeof(server_addr));
    //监听端口，htons函数将主机字节序转换为网络字节序
    server_addr.sin_port=htons(PORT);
    //将服务器IP地址从字符串转换为二进制形式
    if(inet_pton(AF_INET,argv[1],&server_addr.sin_addr)<=0){
        perror("inet_pton failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    //调用connect函数连接服务器，connect函数会阻塞直到连接成功
    if(connect(client_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1){
        perror("connect failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    //打印连接成功信息
    printf("成功连接到服务器 %s:%d\n",argv[1],PORT);
    //收发缓冲区
    char send_buf[BUF_SIZE]={0};
    char recv_buf[BUF_SIZE]={0};
    //持续收发数据
    while(true){
        printf("请输入要发送的数据：");
        //从标准输入读取数据
        fgets(send_buf,BUF_SIZE,stdin);
        //判断输入是否为exit，如果是则退出循环
        if(strncmp(send_buf,"exit",4)==0){
            break;
        }
        //发送数据到服务器，send函数会阻塞直到数据发送完成
        send(client_fd,send_buf,strlen(send_buf),0);
        //接收服务器返回的数据，recv函数会阻塞直到有数据可读
        ssize_t recv_len = recv(client_fd,recv_buf,BUF_SIZE,0);
        //打印服务器返回的数据
        printf("服务器返回的数据：%s",recv_buf);
        //清空缓冲区
        memset(send_buf,0,sizeof(send_buf));
        memset(recv_buf,0,sizeof(recv_buf));
    }
    //关闭套接字
    close(client_fd);
    return 0;
}