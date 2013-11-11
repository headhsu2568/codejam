#include "headin.h"

void doit(int sockfd){
	char name[4096];
	char buf[4096];
	int n;
	if((n=read(sockfd,name,sizeof(name)))!=0){
		int kk=strlen(name);
		name[kk-1]='\0';
		while(1){
			if((n=read(sockfd,buf,sizeof(buf)))==0){
				break;
			}
			FILE* fi=fopen(name,"a+");
			fwrite(buf,1,n,fi);
			fclose(fi);
		}
	}
}

int main(int argc,char** argv){
	int listenfd,connfd;
	pid_t pid;
	char buf[4096];
	struct sockaddr_in servaddr,cliaddr;
	listenfd=socket(AF_INET,SOCK_STREAM,0);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(5566);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	listen(listenfd,1024);
	socklen_t clilen=sizeof(cliaddr);
	while(1){
		connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
		if((pid=fork())==0){
			close(listenfd);
			doit(connfd);
			close(connfd);
			exit(0);
		}
		close(connfd);
	}
	exit(0);
}
