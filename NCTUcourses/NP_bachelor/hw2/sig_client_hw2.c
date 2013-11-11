#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
typedef void Sigfunc(int);

Sigfunc *signal(int signo,Sigfunc *func){
    struct sigaction act,oact;
    act.sa_handler=func;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    if(signo==SIGALRM){
#ifdef SA_INTERRUPT
        act.sa_flags|=SA_INTERRUPT;
#endif
    }
    else{
#ifdef SA_RESTART
        act.sa_flags|=SA_RESTART;
#endif
    }
    if(sigaction(signo,&act,&oact)<0){
        return SIG_ERR;
    }
    return oact.sa_handler;
}

static void sig_alrm(int signo){
    alarm(0);
    return;
}

struct datagram{
    int seq;
    char type[4];
    char payload[4096];
    int plsize;
};

void do_cli(FILE *fp,int sockfd,const struct sockaddr* servaddr,socklen_t servlen){
    int n;
    struct datagram sendline,recvline;
    int seq=-1;
    int resend;
    signal(SIGALRM,sig_alrm);
    char name[4096];
    int iseof=0;
    if(fgets(sendline.payload,4096,fp)!=NULL){
        strcpy(name,sendline.payload);
        sendline.plsize=strlen(sendline.payload);
        name[sendline.plsize-1]='\0';
        FILE *fi;
        fi=fopen(name,"r");
        while(1){
            seq=(seq+1)%2;
            sendline.seq=seq;
            if(iseof){
                bzero(&sendline.type,sizeof(sendline.type));
                sscanf("FIN","%s",&sendline.type);
            }
            do{
                sendto(sockfd,&sendline,sizeof(struct datagram),0,servaddr,servlen);
                resend=0;
                alarm(1);
                if((n=recvfrom(sockfd,&recvline,sizeof(struct datagram),0,NULL,NULL))<=0){
                    if(errno==EINTR){
                        resend=1;
                    }
                    else{
                        printf("recvfrom error!\n");
                        exit(0);
                    }
                }
                alarm(0);
            }while(resend);
            if(iseof){
                break;
            }
            if(feof(fi)){
                iseof=1;
                continue;
            }
            bzero(&recvline.seq,sizeof(recvline.seq));
            bzero(&recvline.type,sizeof(recvline.type));
            bzero(&recvline.payload,sizeof(recvline.payload));
            bzero(&recvline.plsize,sizeof(recvline.plsize));
            bzero(&sendline.seq,sizeof(sendline.seq));
            bzero(&sendline.type,sizeof(sendline.type));
            bzero(&sendline.payload,sizeof(sendline.payload));
            bzero(&sendline.plsize,sizeof(sendline.plsize));
            sendline.plsize=fread(sendline.payload,1,sizeof(sendline.payload),fi);
        }
        fclose(fi);
    }
    return;
}

int main(int argc,char** argv){
    int sockfd;
    struct sockaddr_in servaddr;
    if(argc!=3){
        printf("usage:<IPaddress> <Ports>\n");
        exit(0);
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    int SERV_PORT=atoi(argv[2]);
    servaddr.sin_port=htons(SERV_PORT);
    inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    do_cli(stdin,sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    exit(0);
}
