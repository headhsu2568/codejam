#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>

int main(int argc,char** argv){
    if(argc!=2){
        printf("uasge:2_sa <Port>\n");
        exit(0);
    }
    FILE* ServerA_log;
    int j,i,maxi,maxfd,listenfd,connfd,sockfd;
    int nready,client[FD_SETSIZE];
    ssize_t n;
    fd_set rset,allset;
    char buf[4096];
    socklen_t clilen;
    struct sockaddr_in cliaddr,servaddr;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    int servport=atoi(argv[1]);
    servaddr.sin_port=htons(servport);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    listen(listenfd,1024);
    printf("listening......\n");
    ServerA_log=fopen("ServerA_log.txt","a+");
    fputs("listening......\n",ServerA_log);
    fclose(ServerA_log);
    maxfd=listenfd;
    maxi=-1;
    for(i=0;i<FD_SETSIZE;++i){
        client[i]=-1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd,&allset);
    while(1){
        rset=allset;
        nready=select(maxfd+1,&rset,NULL,NULL,NULL);
        if(FD_ISSET(listenfd,&rset)){
            clilen=sizeof(cliaddr);
            connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
            for(i=0;i<FD_SETSIZE;++i){
                if(client[i]<0){
                    client[i]=connfd;
                    break;
                }
            }
            if(i==FD_SETSIZE){
                printf("too many client!!\n");
                ServerA_log=fopen("ServerA_log.txt","a+");
                fputs("too many client!!\n",ServerA_log);
            }
            else{
                bzero(buf,sizeof(buf));
                sprintf(buf,"!!SYSTEM: welcome client_%d !!!\n",connfd);
                ServerA_log=fopen("ServerA_log.txt","a+");
                fputs(buf,ServerA_log);
                fclose(ServerA_log);
                for(j=0;j<=maxi;++j){
                    if(client[j]<0 || client[j]==connfd){
                        continue;
                    }
                    write(client[j],buf,strlen(buf));
                }
                printf("%s",buf);
                bzero(buf,sizeof(buf));
                FD_SET(connfd,&allset);
                if(connfd>maxfd){
                    maxfd=connfd;
                }
                if(i>maxi){
                    maxi=i;
                }
                if(--nready<=0){
                    continue;
                }
            }
        }

        for(i=0;i<=maxi;++i){
            if((sockfd=client[i])<0){
                continue;
            }
            if(FD_ISSET(sockfd,&rset)){
                sprintf(buf,"client_%d: ",sockfd);
                if((n=read(sockfd,buf+strlen(buf),4096-strlen(buf)))==0){
                    bzero(buf,sizeof(buf));
                    sprintf(buf,"!!SYSTEM: client_%d exit.....\n",sockfd);
                    close(sockfd);
                    FD_CLR(sockfd,&allset);
                    client[i]=-1;
                }
                for(j=0;j<=maxi;++j){
                    if(client[j]<0 || j==i){
                        continue;
                    }
                    write(client[j],buf,strlen(buf));
                }
                printf("%s",buf);
                ServerA_log=fopen("ServerA_log.txt","a+");
                fputs(buf,ServerA_log);
                fclose(ServerA_log);
                bzero(buf,sizeof(buf));
                if(--nready<=0){
                    break;
                }
            }
        }
    }
}
