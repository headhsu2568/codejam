#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>

struct datagram{
    int seq;
    char type[4];
    char payload[4096];
    int plsize;
};

int main(int argc,char** argv){
    if(argc!=2){
        printf("usage:<Port>\n");
        exit(0);
    }
    int sockfd;
    struct sockaddr_in servaddr,cliaddr;
    int len=sizeof(cliaddr);
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    int SERV_PORT=atoi(argv[1]);
    servaddr.sin_port=htons(SERV_PORT);
    bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    int n;
    int cur_seq=-1;
    struct datagram mesg;
    char name[4096];
    while(1){
        n=recvfrom(sockfd,&mesg,sizeof(struct datagram),0,(struct sockaddr*)&cliaddr,&len);
        srand(time(NULL));
        float p;
        p=(rand()%100)*0.01;
        if (p<0.1){
            continue;
        }
        else{
            if(cur_seq!=mesg.seq){
                if(strcmp(mesg.type,"FIN")==0){
                    printf("Client terminated.\n");
                }
                else if(!strlen(name)){
                    strcpy(name,mesg.payload);
                    int namelen=strlen(name);
                    name[namelen-1]='\0';
                }
                else{
                    FILE *fi=fopen(name,"a+");
                    fwrite(mesg.payload,1,mesg.plsize,fi);
                    fclose(fi);
                }
                cur_seq=mesg.seq;
                mesg.seq=(mesg.seq+1)%2;
            }
            srand(time(NULL));
            p=(rand()%100)*0.01;
            sscanf("ACK","%s",&mesg.type);
            if(p>0.1){
                sendto(sockfd,&mesg,n,0,(struct sockaddr*)&cliaddr,len);
            }
            bzero(&mesg.payload,sizeof(mesg.payload));
            bzero(&mesg.type,sizeof(mesg.type));
            bzero(&mesg.seq,sizeof(mesg.seq));
            bzero(&mesg.plsize,sizeof(mesg.plsize));
        }
    }
    return 0;
}
