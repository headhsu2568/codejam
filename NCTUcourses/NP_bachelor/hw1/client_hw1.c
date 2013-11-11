#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "str_cli_hw1.c"

int main(int argc,char** argv){
    if(argc!=2){
        perror("usage:a.out <IPaddress>");
        exit(1);
    }
    int sockfd;
    char recv[4096];
    char send[4096];
    struct sockaddr_in servaddr;
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
        perror("socket error!");
        exit(1);
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(5566);
    if(inet_pton(AF_INET,argv[1],&servaddr.sin_addr)<=0){
        printf("inet_pton error for %s",argv[1]);
        exit(1);
    }
    if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        perror("connect error!");
        exit(1);
    }
    str_cli(stdin,sockfd);
    printf("BYE~\n");
    exit(0);
    return 0;
}
