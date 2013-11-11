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

int main(int argc,char** argv){
    if(argc!=3){
        printf("usage:<IP address> <Port>\n");
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
    int n;
    signal(SIGALRM,sig_alrm);
    char *HW="Hello world";
    char buf[4096];
    while(1){
        sendto(sockfd,HW,strlen(HW)+1,0,(struct sockaddr*)&servaddr,sizeof(servaddr));
        alarm(1);
        if((n=recvfrom(sockfd,buf,sizeof(buf),0,NULL,NULL))<0){
            if(errno==EINTR){
                continue;
            }
            else{
                printf("recvfrom error!\n");
                exit(0);
            }
        }
        else{
            printf("%s\n",buf);
        }
        bzero(buf,sizeof(buf));
        sleep(1000);
        alarm(0);
    }
}
