#include "headin.h"

int main(int argc,char** argv){
	int listenfd,connfd,j,i,maxi,maxfd,sockfd;
	char buf[4096];
	struct sockaddr_in servaddr,cliaddr;
	int nready,client[FD_SETSIZE];
	ssize_t n;
	char c_str[FD_SETSIZE][4096];
	fd_set rset,allset;
	socklen_t clilen=sizeof(cliaddr);
	listenfd=socket(AF_INET,SOCK_STREAM,0);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(5566);
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	listen(listenfd,1024);
	maxfd=listenfd;
	maxi=-1;
	for(i=0;i<FD_SETSIZE;++i){
		client[i]=-1;
	}
	FD_ZERO(&allset);
	FD_SET(listenfd,&allset);
	while(1){
		rset=allset;
		nready=select(maxfd+1,&rset,NULL,NULL,NULL);
		if(FD_ISSET(listenfd,&rset)){
			connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&clilen);
			for(i=0;i<FD_SETSIZE;++i){
				if(client[i]==-1){
					client[i]=connfd;
					break;
				}
			}
			if(i==FD_SETSIZE){
				printf("too many client!\n");
			}
			else{
				FD_SET(client[i],&allset);
				if(connfd>maxfd){
					maxfd=connfd;
				}
				if(i>maxi){
					maxi=i;
				}
			}
			if(--nready<=0){
				continue;
			}
		}
		for(i=0;i<=maxi;++i){
			if(client[i]<0){
				continue;
			}
			int pp=client[i];
			if(FD_ISSET(client[i],&rset)){
				if((n=read(client[i],c_str[i],sizeof(c_str[i])))==0){
					close(client[i]);
					FD_CLR(client[i],&allset);
					client[i]=-1;
					bzero(c_str[i],sizeof(c_str[i]));
				}
				else{
						int kkk=strlen(c_str[i]);
						c_str[i][kkk-1]='\0';
//					while(1){
						if((n=read(client[i],buf,sizeof(buf)))==0){
							break;
						}
						FILE* fi=fopen(c_str[i],"a+");
						fwrite(buf,1,n,fi);
						fclose(fi);
						bzero(buf,sizeof(buf));
//					}
				}
				if(--nready<=0){
					break;
				}
			}
		}
	}
	exit(0);
}
