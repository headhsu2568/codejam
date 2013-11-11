#include "headin.h"

void str_cli(FILE* fp,int sockfd){
	char buf[4096];
	char name[4096];
	char tmp_name[4096];
	if(fgets(name,sizeof(name),fp)!=NULL){
		strcpy(tmp_name,name);
		int ppp=strlen(name);
		name[ppp]='\0';
		write(sockfd,name,sizeof(name));
		FILE *fi;
		int kkk=strlen(tmp_name);
		tmp_name[kkk-1]='\0';
		fi=fopen(tmp_name,"r");
		while(1){
		
			int nn=fread(buf,1,sizeof(buf),fi);
			write(sockfd,buf,nn);
			bzero(buf,sizeof(buf));
if(feof(fi)){
			break;
		}
		}
		fclose(fi);
	}
}

int main(int argc,char** argv){
	if(argc!=2){
		printf("usage:a.out <IPaddress>\n");
		exit(0);
	}
	int sockfd;
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
