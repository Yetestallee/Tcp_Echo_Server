#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define PORT 8888
#define BUF_SIZE 1024

int main(){
    //创建tcp套接字
    int server_fd = socket(AF_INET,SOCK_STREAM,0);
    //检查是否创建成功
    if(server_fd==-1){
        perror("socker failed");
        exit(EXIT_FAILURE);
    }
    //配置服务器
    struct sockaddr_in server_addr;
    //设置为IPV4地址
    server_addr.sin_family=AF_INET;
    //监听端口，htons函数将主机字节序转换为网络字节序
    server_addr.sin_port=htons(PORT);
    //监听所有接口，INADDR_ANY表示接受任何IP地址的连接
    server_addr.sin_addr.s_addr=INADDR_ANY;
    //将sin_zero字段清零，sin_zero是一个填充字段，保持结构体大小一致
    memset((void*)server_addr.sin_zero,0,sizeof(server_addr.sin_zero));
    //绑定套接字到地址和端口
    if(bind(server_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    //监听连接，参数5表示最大等待连接数
    if(listen(server_fd,5)==-1){
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    //打印服务器监听信息
    printf("Server is listening on port %d\n",PORT);
    //定义结构体来存储客户端地址信息
    struct sockaddr_in client_addr;
    socklen_t client_addr_len=sizeof(client_addr);
    //接受客户端连接，accept函数会阻塞直到有客户端连接
    int client_fd = accept(server_fd,(struct sockaddr*)&client_addr,&client_addr_len);
    //检查是否接受成功
    if(client_fd==-1){
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    //打印客户端连接信息
    printf("Client connected: %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
    //定义缓冲区来存储接收的数据
    char buffer[BUF_SIZE]={0};
    //接收数据，recv函数会阻塞直到有数据可读
    while(true){
        ssize_t recv_len = recv(client_fd,buffer,BUF_SIZE,0);
        //检查是否接收成功
        if(recv_len<=0){
            printf("客户端断开连接\n");
            break;
        }
        //打印接收到的数据
        printf("Received from client: %s\n",buffer);
        //发送响应，send函数将数据发送回客户端
        send(client_fd,(const void*)buffer,recv_len,0);
        //清空缓冲区
        memset(buffer,0,sizeof(buffer));
    }
    //关闭套接字
    close(client_fd);
    close(server_fd);
    return 0;
}