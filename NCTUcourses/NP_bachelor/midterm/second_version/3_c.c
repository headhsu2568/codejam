#include "headin.h"

//void str_cli(FILE* fp,int sockfd,char* name){
void str_cli(FILE* fp,int sockfd){
    char name[4096];
    char name_2[4096];
    char buf[4096];
    int len;
    if(fgets(name,sizeof(name),fp)!=NULL){
        strcpy(name_2,name);
        len=strlen(name);
        name[len-1]='\0';
//        len=strlen(name);
//        strcpy(name_2,name);
//        name_2[len]='\n';
//        name_2[len+1]='\0';
        FILE* fi=fopen(name,"a+");
        while(fgets(buf,sizeof(buf),fp)!=NULL){
           fputs(buf,fi);
           bzero(buf,sizeof(buf));
        }
        fclose(fi);
        printf("Create file complete!\n");
        write(sockfd,name_2,sizeof(name_2));
        printf("Start transmission!\n");
        FILE* fi_2=fopen(name,"r");
        while(1){
            len=fread(buf,1,sizeof(buf),fi_2);
            write(sockfd,buf,len);
            bzero(buf,sizeof(buf));
            if(feof(fi_2)){
                printf("read finish!\n");
                break;
            }
        }
        fclose(fi_2);
        printf("Transmission complete!\n");
    }
}

int main(int argc,char** argv){
//    if(argc!=4){
//        printf("usage:3_c <IPaddress> <Port> <FileName>\n");
//        exit(0);
//    }
    if(argc!=3){
         printf("usage:3_c <IPaddress> <Port>\n");
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
//    str_cli(stdin,sockfd,argv[3]);
}
