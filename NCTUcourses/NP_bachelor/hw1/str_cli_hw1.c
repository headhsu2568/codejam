#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
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
