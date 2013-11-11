#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>

void do_cli(FILE *fp,int sockfd,const struct sockaddr* servaddr,socklen_t servlen){
    int n;
    char recvline[8192],sendline[4096];
    int maxfd;
    fd_set rset,allset;
    FD_ZERO(&allset);
    FD_ZERO(&rset);
    FD_SET(fileno(fp),&allset);
    FD_SET(sockfd,&allset);
    if(sockfd>fileno(fp)){
        maxfd=sockfd;
    }
    else{
        maxfd=fileno(fp);
    }
    while(1){
        FD_ZERO(&rset);
        rset=allset;
        int nready=select(maxfd+1,&rset,NULL,NULL,NULL);
        if(FD_ISSET(fileno(fp),&rset)){
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
        }
        if(FD_ISSET(sockfd,&rset)){
            if((n=recvfrom(sockfd,recvline,sizeof(recvline),0,NULL,NULL))<=0){
                printf("server terminated prematurely.\n");
                exit(0);
            }
            printf("output:%s\n",recvline);
            bzero(recvline,sizeof(recvline));
        }
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
