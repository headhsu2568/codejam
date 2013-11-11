#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>

struct datagram{
    int seq;
    char type[4];
    char payload[4096];
    int plsize;
};

int readable_timeo(int fd,int sec){
    fd_set rset;
    struct timeval tv;
    FD_ZERO(&rset);
    FD_SET(fd,&rset);
    tv.tv_sec=sec;
    tv.tv_usec=0;
    return (select(fd+1,&rset,NULL,NULL,&tv));
}

void do_cli(FILE *fp,int sockfd,const struct sockaddr* servaddr,socklen_t servlen){
    int n;
    struct datagram sendline,recvline;
    int seq=-1;
    int resend;
    int iseof=0;
    char name[4096];
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
                if(readable_timeo(sockfd,1)==0){
                    resend=1;
                }
                else{
                    n=recvfrom(sockfd,&recvline,sizeof(struct datagram),0,NULL,NULL);
                }
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
