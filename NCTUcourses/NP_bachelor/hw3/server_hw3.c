#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>
#include <fcntl.h>
#include <sys/errno.h>

char* gf_time(){
    struct timeval tv;
    static char str[30];
    char* ptr;
    if(gettimeofday(&tv,NULL)<0){
        printf("gettimeofday error\n");
        exit(0);
    }
    ptr=ctime(&tv.tv_sec);
    strcpy(str,&ptr[11]);
    snprintf(str+8,sizeof(str)-8,".%06ld",tv.tv_usec);
    return str;
}

int main(int argc,char** argv){
    if(argc!=2){
        printf("usage:<Port>\n");
        exit(0);
    }
    int val,i,maxi,maxfd,listenfd,connfd,SERV_PORT;
    int nready,client[FD_SETSIZE];
    unsigned int len;
    ssize_t n,nwritten;
    fd_set rset,wset,allset;
    char buf[FD_SETSIZE][8192];
    char* iptr[FD_SETSIZE],*optr[FD_SETSIZE];
    socklen_t clilen;
    struct sockaddr_in cliaddr,servaddr;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    SERV_PORT=atoi(argv[1]);
    servaddr.sin_port=htons(SERV_PORT);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    listen(listenfd,1024);
    printf("listening...\n");
    maxfd=listenfd;
    maxi=-1;
    for(i=0;i<FD_SETSIZE;++i){
        client[i]=-1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd,&allset);
    while(1){
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        FD_SET(listenfd,&rset);
        for(i=0;i<FD_SETSIZE;++i){
            if(client[i]>=0){
                if((iptr[i]==optr[i])&&(iptr[i]==&buf[i][8192])){
                    iptr[i]=optr[i]=buf[i];
                }
                if(iptr[i]<&buf[i][8192]){
                    FD_SET(client[i],&rset);
                }
                if(iptr[i]!=optr[i]){
                    FD_SET(client[i],&wset);
                }
            }
        }
        select(maxfd+1,&rset,&wset,NULL,NULL);
        if(FD_ISSET(listenfd,&rset)){
            clilen=sizeof(cliaddr);
            connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
            for(i=0;i<FD_SETSIZE;++i){
                if(client[i]<0){
                    client[i]=connfd;
                    val=fcntl(client[i],F_GETFL,0);
                    fcntl(client[i],F_SETFL,val|O_NONBLOCK);
                    iptr[i]=buf[i];
                    optr[i]=buf[i];
                    printf("welcome client %d\n",i+1);
                    break;
                }
            }
            if(i==FD_SETSIZE){
                printf("error:too many client!\n");
            }
            FD_SET(connfd,&allset);
            if(connfd>maxfd){
                maxfd=connfd;
            }
            if(i>maxi){
                maxi=i;
            }
        }
        for(i=0;i<=maxi;++i){
            if(client[i]<0){
                continue;
            }
            if(FD_ISSET(client[i],&rset)){
                if((n=read(client[i],iptr[i],&buf[i][8192]-iptr[i]))<0){
                    if(errno!=EWOULDBLOCK){
                        printf("read error on client %d!\n",i+1);
                        exit(0);
                    }
                }
                else if(n==0){
                    printf("client %d terminated\n",i+1);
                    close(client[i]);
                    client[i]=-1;
                }
                else{
                    printf("%s: read %d bytes from client %d\n",gf_time(),n,i+1);
                    iptr[i]+=n;
                    FD_SET(client[i],&wset);
                }
            }
        }
        for(i=0;i<=maxi;++i){
            if(client[i]<0){
                continue;
            }
            if(FD_ISSET(client[i],&wset) && (iptr[i]-optr[i])>0){
                if((nwritten=write(client[i],optr[i],iptr[i]-optr[i]))<0){
                    if(errno!=EWOULDBLOCK){
                        printf("write error to client %d!\n",i+1);
                        exit(0);
                    }
                }
                else{
                    printf("%s: wrote %d bytes to client %d\n",gf_time(),nwritten,i+1);
                    optr[i]+=nwritten;
                    if(optr[i]==iptr[i]){
                        optr[i]=buf[i];
                        iptr[i]=buf[i];
                    }
                }
            }
        }
    }
}
