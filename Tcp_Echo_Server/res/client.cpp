#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define PORT 8888
#define BUF_SIZE 1024

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
    //配置服务器
    struct sockaddr_in server_addr;
    //设置为IPV4地址
    server_addr.sin_family=AF_INET;
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