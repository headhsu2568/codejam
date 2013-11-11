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

void str_cli(FILE* fp,int sockfd){
    char recv[4096];
    char send[4096];
    struct arg in;
    struct result out;
    while(fgets(send,4096,fp)!=NULL){
        if(sscanf(send,"%d%d%d%d",&in.arg1,&in.arg2,&in.arg3,&in.arg4)!=4){
            printf("input error!\n");
            continue;
        }
        write(sockfd,&in,sizeof(in));
        if(read(sockfd,&out,sizeof(out))<=0){
            printf("Server terminated prematurely!\n");
            return;
        }
        printf("1:%d+%d=%d\n",in.arg1,in.arg2,out.value1);
        printf("2:%d+%d=%d\n",in.arg3,in.arg4,out.value2);
    }
}

int main(int argc,char** argv){
    if(argc!=3){
        printf("usage:1_c <IPaddress> <Port>\n");
        exit(0);
    }
    int sockfd;
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    int servport=atoi(argv[2]);
    servaddr.sin_port=htons(servport);

    if(inet_pton(AF_INET,argv[1],&servaddr.sin_addr)<=0){
		printf("inet_pton error for %s",argv[1]);
		exit(0);
	}
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		printf("socket error\n");
		exit(0);
	}
	if((connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)))<0){
		printf("connect error\n");
		exit(0);
	}
    str_cli(stdin,sockfd);
    exit(0);
}
