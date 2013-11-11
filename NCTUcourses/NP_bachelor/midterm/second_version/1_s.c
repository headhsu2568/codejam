#include "headin.h"

struct arg{
    int arg1;
    int arg2;
    int arg3;
    int arg4;
};

struct result{
    int value1;
    int value2;
};

int main(int argc,char **argv){
    if(argc!=2){
        printf("usage:1_s <port>\n");
        exit(0);
    }
    int listenfd,connfd;
    struct arg in;
    struct result out;
    struct sockaddr_in servaddr,cliaddr;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    int servport=atoi(argv[1]);
    servaddr.sin_port=htons(servport);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    listen(listenfd,1024);
    printf("listening....\n");
    socklen_t clilen=sizeof(cliaddr);
    if((connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen))<=0){
        printf("accept error for %d!\n",connfd);
        exit(0);
    }
    printf("client request...\n");
    while(1){
        if(read(connfd,&in,sizeof(in))==0){
            printf("Client has closed the connect!\n");
            close(connfd);
            break;
        }
        out.value1=in.arg1+in.arg2;
        out.value2=in.arg3+in.arg4;
        printf("1:%d+%d=%d\n",in.arg1,in.arg2,out.value1);
        printf("2:%d+%d=%d\n",in.arg3,in.arg4,out.value2);
        write(connfd,&out,sizeof(out));
    }
}
