#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

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
            do{
                sendto(sockfd,&sendline,sizeof(struct datagram),0,servaddr,servlen);
                printf("send: %d: %s",sendline.seq,sendline.payload);
                resend=0;
                //timer start
                n=recvfrom(sockfd,&recvline,sizeof(struct datagram),0,NULL,NULL);
                //timeout
                if(recvline.type=="FIN"){
                    printf("Server terminated prematurely.\n");
                    exit(0);
                }
                if(recvline.seq==seq){
                    resend=1;
                }
                printf("%s %d :%s",recvline.type,recvline.seq,recvline.payload);
            }while(resend);
            if(feof(fi) || recvline.type=="FIN"){
                break;
            }
            bzero(&sendline.seq,sizeof(sendline.seq));
            bzero(&sendline.type,sizeof(sendline.type));
            bzero(&sendline.payload,sizeof(sendline.payload));
            bzero(&sendline.plsize,sizeof(sendline.plsize));
            sendline.plsize=fread(sendline.payload,1,sizeof(sendline.payload),fi);
        }
        fclose(fi);
    }
    seq=(seq+1)%2;
    sendline.seq=seq;
    bzero(&sendline.type,sizeof(sendline.type));
    sscanf("FIN","%s",&sendline.type);
    sendto(sockfd,&sendline,sizeof(struct datagram),0,servaddr,servlen);
    //    printf("%s %d\n",sendline.type,sendline.seq);
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
