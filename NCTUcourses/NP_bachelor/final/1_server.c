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
    char *ACK="ACK";
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
    while(1){
        char input1[4096];
        char input2[4096];
        char output[8192];
        n=recvfrom(sockfd,input1,sizeof(input1),0,(struct sockaddr*)&cliaddr,&len);
        input1[strlen(input1)-1]='\0';
        printf("input1:%s\n",input1);
        sendto(sockfd,ACK,sizeof(ACK),0,(struct sockaddr*)&cliaddr,len);
        n=recvfrom(sockfd,input2,sizeof(input2),0,(struct sockaddr*)&cliaddr,&len);
        input2[strlen(input2)-1]='\0';
        printf("input2:%s\n",input2);
        sendto(sockfd,ACK,sizeof(ACK),0,(struct sockaddr*)&cliaddr,len);
        strcpy(output,input1);
        strcat(output,input2);
        printf("output:%s\n",output);
        sendto(sockfd,output,sizeof(output),0,(struct sockaddr*)&cliaddr,len);
    }
}
