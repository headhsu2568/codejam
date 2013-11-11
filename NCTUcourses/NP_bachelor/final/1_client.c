#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void do_cli(FILE *fp,int sockfd,const struct sockaddr* servaddr,socklen_t servlen){
    int n;
    char recvline[8192],sendline[4096];
    while(1){
        printf("input1:");
        if((fgets(sendline,sizeof(sendline),fp))!=NULL){
            sendto(sockfd,sendline,sizeof(sendline),0,servaddr,servlen);
            if((n=recvfrom(sockfd,recvline,sizeof(recvline),0,NULL,NULL))<=0){
                printf("server terminated prematurely.\n");
                exit(0);
            }
            printf("%s\n",recvline);
        }
        bzero(sendline,sizeof(sendline));
        bzero(recvline,sizeof(recvline));
        printf("input2:");
        if((fgets(sendline,sizeof(sendline),fp))!=NULL){
            sendto(sockfd,sendline,sizeof(sendline),0,servaddr,servlen);
            if((n=recvfrom(sockfd,recvline,sizeof(recvline),0,NULL,NULL))<=0){
                printf("server terminated prematurely.\n");
                exit(0);
            }
            printf("%s\n",recvline);
        }
        bzero(sendline,sizeof(sendline));
        bzero(recvline,sizeof(recvline));
        if((n=recvfrom(sockfd,recvline,sizeof(recvline),0,NULL,NULL))<=0){
            printf("server terminated prematurely.\n");
            exit(0);
        }
        printf("%s\n",recvline);
        bzero(recvline,sizeof(recvline));
    }
}

int main(int argc,char** argv){
    if(argc!=3){
        printf("usage:<IPaddress> <Port>\n");
        exit(0);
    }
    int sockfd;
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    int SERV_PORT=atoi(argv[2]);
    servaddr.sin_port=htons(SERV_PORT);
    inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    do_cli(stdin,sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    exit(0);
}
