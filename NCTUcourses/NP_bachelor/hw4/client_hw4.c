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
    int part;
    int sockfd;
    int port;
    char* ip;
    int size;
};

static void *do_cli(void *s){
    char buf[4096];
    struct sockaddr_in servaddr;
    struct thread_info *t=(struct thread_info*)s;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(t->port);
    inet_pton(AF_INET,t->ip,&servaddr.sin_addr);
    connect(t->sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    char command[4096];
    sprintf(command,"Want to download the %dth 1/5 part of a requested file",t->part);
    printf("command:%s\n",command);
    write(t->sockfd,command,sizeof(command));
    char name[4096];
    sprintf(name,"tmp_%d",t->part);
    printf("name:%s\n",name);
    FILE* fp=fopen(name,"w");
    int n;
    while((n=read(t->sockfd,buf,sizeof(buf)))>0){
        printf("thread_%d read %d\n",t->part,n);
        fwrite(buf,1,n,fp);
        bzero(buf,sizeof(buf));
    }
    fclose(fp);
    free(s);
    pthread_exit(0);
}

void merge(){
    char *name="new";
    char tmp_name[4096];
    char buf[4096];
    FILE* fp=fopen(name,"w");
    int i=1;
    for(i=1;i<6;++i){
        sprintf(tmp_name,"tmp_%d",i);
        FILE* file=fopen(tmp_name,"r");
        while(1){
            int nn=fread(buf,1,sizeof(buf),file);
            fwrite(buf,1,nn,fp);
            bzero(buf,sizeof(buf));
            if(feof(file)){
                break;
            }
        }
        fclose(file);
    }
    fclose(fp);
}

void str_cli(FILE *fp,int sockfd,int port,char* ip){
    char recvline[4096],sendline[4096];
    int size;
    if(fgets(sendline,4096,fp)!=NULL){
        write(sockfd,sendline,sizeof(sendline));
        int n=read(sockfd,recvline,sizeof(recvline));
        printf("%s\n",recvline);
        size=atoi(recvline);
    }
    int i;
    pthread_t tid[5];
    struct thread_info *t;
    for(i=1;i<6;++i){
        t=(struct thread_info*)malloc(sizeof(struct thread_info));
        t->sockfd=socket(AF_INET,SOCK_STREAM,0);
        t->part=i;
        t->port=port+i;
        t->ip=ip;
        t->size=size;
        pthread_create(&tid[i-1],NULL,&do_cli,(void*)t);
    }
    for(i=0;i<5;++i){
        pthread_join(tid[i],NULL);
    }
    merge();
    printf("GET complete!\n");
}

int main(int argc,char** argv){
    int sockfd,SERV_PORT;
    struct sockaddr_in servaddr;
    if(argc!=3){
        printf("usage:<IPaddress> <Port\n>");
        exit(0);
    }
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    SERV_PORT=atoi(argv[2]);
    servaddr.sin_port=htons(SERV_PORT);
    inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
    connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    str_cli(stdin,sockfd,SERV_PORT,argv[1]);
    exit(0);
}
