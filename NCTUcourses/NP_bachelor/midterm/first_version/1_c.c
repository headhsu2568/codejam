#include "headin.h"

struct arg{
	int arg1;
	int arg2;
	int arg3;
};

struct result{
	int max;
	int min;
};

void str_cli(FILE* fp,int sockfd){
	char recv[4096];
	char send[4096];
	struct arg in;
	struct result out;
	while(fgets(send,4096,fp)!=NULL){
		if(sscanf(send,"%d%d%d",&in.arg1,&in.arg2,&in.arg3)!=3){
			printf("input error\n");
			continue;
		}
		write(sockfd,&in,sizeof(in));
		if(read(sockfd,&out,sizeof(out))==0){
			printf("Server terminated prematurely\n");
			return;
		}
		printf("max is %d\n",out.max);
		printf("min is %d\n",out.min);
	}
}

int main(int argc,char** argv){
	if(argc!=2){
		printf("usage:a.out <IPaddress>\n");
		exit(0);
	}
	int sockfd;
	char recv[4096];
	char send[4096];
	struct sockaddr_in servaddr;
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(5566);
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
