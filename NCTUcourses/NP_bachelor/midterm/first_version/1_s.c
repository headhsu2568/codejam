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

int main(int argc,char** argv){
	int listenfd,connfd;
	struct arg in;
	struct result out;
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
	connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
	while(1){
		if(read(connfd,&in,sizeof(in))==0){
			printf("Client has closed the connect\n");
			close(connfd);
			break;
		}
		if(in.arg1>=in.arg2 && in.arg1>=in.arg3){
			out.max=in.arg1;
		}
		else if(in.arg2>=in.arg1 && in.arg2>=in.arg3){
			out.max=in.arg2;
		}
		else if(in.arg3>=in.arg1 && in.arg3>=in.arg2){
			out.max=in.arg3;
		}
		if(in.arg1<=in.arg2 && in.arg1<=in.arg3){
			out.min=in.arg1;
		}
		else if(in.arg2<=in.arg1 && in.arg2<=in.arg3){
			out.min=in.arg2;
		}
		else if(in.arg3<=in.arg1 && in.arg3<=in.arg2){
			out.min=in.arg3;
		}
		write(connfd,&out,sizeof(out));
	}
	exit(0);
}
