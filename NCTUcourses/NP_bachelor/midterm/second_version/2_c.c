#include "headin.h"
#ifndef max
    #define max(a,b) ( ((a)>(b)) ? (a) : (b) )
#endif

void str_cli(FILE* fp,int sockfd){
    int maxfdp1,stdineof;
    fd_set rset;
    char buf[4096];
    int n;

    stdineof=0;
    FD_ZERO(&rset);
    while(1){
        if(stdineof==0){
            FD_SET(fileno(fp),&rset);
        }
        FD_SET(sockfd,&rset);
        maxfdp1=max(fileno(fp),sockfd)+1;
        select(maxfdp1,&rset,NULL,NULL,NULL);
        if(FD_ISSET(sockfd,&rset)){
            if((n=read(sockfd,buf,sizeof(buf)))<=0){
                if(stdineof==1){
                    return;
                }
                else{
                    perror("str_cli:server terminated prematurely!");
                    exit(1);
                }
            }
            printf("%s",buf);
            bzero(buf,sizeof(buf));
        }
        if(FD_ISSET(fileno(fp),&rset)){
            if((n=read(fileno(fp),buf,4096))==0){
                stdineof=1;
                shutdown(sockfd,SHUT_WR);
                FD_CLR(fileno(fp),&rset);
                continue;
            }
            write(sockfd,buf,n);
            bzero(buf,sizeof(buf));
        }
    }
    return;
}
int main(int argc,char** argv){
    if(argc!=3){
        perror("usage:2_c <IPaddress> <Port>\n");
        exit(1);
    }
    int sockfd;
    char recv[4096];
    char send[4096];
    struct sockaddr_in servaddr;
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
        printf("socket error!\n");
        exit(1);
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    int servport=atoi(argv[2]);
    servaddr.sin_port=htons(servport);
    if(inet_pton(AF_INET,argv[1],&servaddr.sin_addr)<=0){
        printf("inet_pton error for %s\n",argv[1]);
        exit(1);
    }
    if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        perror("connect error!\n");
        exit(1);
    }
    str_cli(stdin,sockfd);
    printf("BYE~\n");
    exit(0);
    return 0;
}
