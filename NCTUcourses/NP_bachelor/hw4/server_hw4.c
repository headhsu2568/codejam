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
#include <pthread.h>

struct thread_info{
    int port;
    char* name;
    int size;
};

static void *doit(void *s){
    struct thread_info *t;
    t=(struct thread_info*)s;
    int listenfd,connfd;
    char buf[4096];
    struct sockaddr_in servaddr,cliaddr;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(t->port);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    listen(listenfd,1024);
    socklen_t clilen=sizeof(cliaddr);
    connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
    read(connfd,buf,sizeof(buf));
    int part;
    sscanf(buf,"Want to download the %dth 1/5 part of a requested file",&part);
    FILE* fp=fopen(t->name,"r");
    printf("open:%s\n",t->name);
    int remain_size;
    if(part==5){
        remain_size=(t->size)/5+(t->size)%5;
    }
    else{
        remain_size=(t->size)/5;
    }
    printf("remain_size:%d\n",remain_size);
    fseek(fp,(part-1)*(t->size/5),SEEK_SET);
    printf("thread_%d tell:%d\n",part,ftell(fp));
    while(1){
        int nn=fread(buf,1,sizeof(buf),fp);
//        printf("thread_%d read:%d\n",part,nn);
        if(remain_size>nn){
            write(connfd,buf,nn);
        }
        else{
//            buf[remain_size]='\0';
            write(connfd,buf,remain_size);
            break;
        }
        remain_size-=nn;
        bzero(buf,sizeof(buf));
    }
    fclose(fp);
    free(s);
    pthread_detach(pthread_self());
    pthread_exit(0);
}

int main(int argc,char** argv){
    if(argc!=2){
        printf("usage:<Port>\n");
        exit(0);
    }
    int listenfd,connfd,SERV_PORT,n;
    char buf[4096];
    struct sockaddr_in servaddr,cliaddr;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    SERV_PORT=atoi(argv[1]);
    servaddr.sin_port=htons(SERV_PORT);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    listen(listenfd,1024);
    socklen_t clilen=sizeof(cliaddr);
    connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
    char name[4096];
    int size,i;
    pthread_t tid[5];
    if((n=read(connfd,buf,sizeof(buf)))>0){
        sscanf(&buf[4],"%s",name);
        FILE *file=fopen(name,"r");
        long b_offset=ftell(file);
        fseek(file,0,SEEK_END);
        long e_offset=ftell(file);
        size=e_offset-b_offset;
        printf("size:%d\n",size);
        bzero(buf,sizeof(buf));
        sprintf(buf,"%d",size);
        struct thread_info* t;
        for(i=1;i<6;++i){
            t=(struct thread_info*)malloc(sizeof(struct thread_info));
            t->port=SERV_PORT+i;
            t->name=name;
            t->size=size;
            pthread_create(&tid[i-1],NULL,&doit,(void *)t);
        }
        write(connfd,buf,sizeof(buf));
        printf("%s\n",buf);
        bzero(buf,sizeof(buf));
        fclose(file);
    }
    for(i=0;i<5;++i){
        pthread_join(tid[i],NULL);
    }
}
