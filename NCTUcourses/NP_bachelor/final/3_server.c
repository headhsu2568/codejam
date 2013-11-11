#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

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
    char buf[4096];
    while(1){
        n=recvfrom(sockfd,buf,sizeof(buf),0,(struct sockaddr*)&cliaddr,&len);
        printf("%s\n",buf);
        sendto(sockfd,buf,sizeof(buf),0,(struct sockaddr*)&cliaddr,len);
        bzero(buf,sizeof(buf));
    }
}
