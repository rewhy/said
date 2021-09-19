/*************************************************************************
	> File Name: server.cpp
	> Author: 
	> Mail: 
	> Created Time: 2019年11月02日 星期六 18时00分03秒
 ************************************************************************/

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>

#define PORT 9999
#define BUFFLEN 1024

void handle_udp_msg(int fd){

    char buff[BUFFLEN];
    socklen_t len;
    int count;
    struct sockaddr_in client_addr;
    while(1){
        memset(buff,0,BUFFLEN);
        len = sizeof(client_addr);
        count = recvfrom(fd,buff,BUFFLEN,0,(struct sockaddr*)&client_addr,&len);
        if(count == -1){
            printf("receive data fail!\n");
            return;
        }
        printf("client:%s\n",buff);
        memset(buff,0,BUFFLEN);
        sprintf(buff, "I have received %d bytes data!\n",count);
        printf("server:%s\n",buff);
        sendto(fd,buff,BUFFLEN,0,(struct sockaddr*)&client_addr,len);
    }

}

int main(){
    int server_fd , ret;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET,SOCK_DGRAM,0);
    if(server_fd<0){
        printf("Create socket fail\n");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址，需要进行网络序转换，INADDR_ANY：本地地址
    server_addr.sin_port = htons(PORT);  //端口号，需要网络序转换

    ret = bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        printf("socket bind fail!\n");
        return -1;
    }

    handle_udp_msg(server_fd);   //处理接收到的数据

    close(server_fd);

    return 0;
}

