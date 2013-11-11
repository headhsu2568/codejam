#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>
#include <fcntl.h>
#include <sys/errno.h>

char* gf_time(){
    struct timeval tv;
    static char str[30];
    char* ptr;
    if(gettimeofday(&tv,NULL)<0){
        printf("gettimeofday error\n");
        exit(0);
    }
    ptr=ctime(&tv.tv_sec);
    strcpy(str,&ptr[11]);
    snprintf(str+8,sizeof(str)-8,".%06ld",tv.tv_usec);
    return str;
}

void str_cli(FILE *fp,int sockfd){
    int maxfdp1,val,stdineof;
    ssize_t n,nwritten;
    fd_set rset,wset;
    char to[8192],fr[8192];
    char *toiptr,*tooptr,*friptr,*froptr;
    val=fcntl(sockfd,F_GETFL,0);
    fcntl(sockfd,F_SETFL,val|O_NONBLOCK);
    val=fcntl(STDIN_FILENO,F_GETFL,0);
    fcntl(STDIN_FILENO,F_SETFL,val|O_NONBLOCK);
    val=fcntl(STDOUT_FILENO,F_GETFL,0);
    fcntl(STDOUT_FILENO,F_SETFL,val|O_NONBLOCK);
    toiptr=to;
    tooptr=to;
    friptr=fr;
    froptr=fr;
    stdineof=0;
    maxfdp1=STDIN_FILENO>STDOUT_FILENO?STDIN_FILENO:STDOUT_FILENO;
    maxfdp1=sockfd>maxfdp1?sockfd:maxfdp1;
    maxfdp1+=1;
    for(;;){
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        if(stdineof==0 && toiptr<&to[8192]){
            FD_SET(STDIN_FILENO,&rset);
        }
        if(friptr<&fr[8192]){
            FD_SET(sockfd,&rset);
        }
        if(tooptr!=toiptr){
            FD_SET(sockfd,&wset);
        }
        if(froptr!=friptr){
            FD_SET(STDOUT_FILENO,&wset);
        }
        select(maxfdp1,&rset,&wset,NULL,NULL);
        if(FD_ISSET(STDIN_FILENO,&rset)){
            if((n=read(STDIN_FILENO,toiptr,&to[8192]-toiptr))<0){
                if(errno!=EWOULDBLOCK){
                    printf("read error on stdin!\n");
                    exit(0);
                }
            }
            else if(n==0){
                fprintf(stderr,"%s: EOF on stdin\n",gf_time());
                stdineof=1;
                if(tooptr==toiptr){
                    shutdown(sockfd,SHUT_WR);
                }
            }
            else{
                fprintf(stderr,"%s: read %d bytes from stdin\n",gf_time(),n);
                toiptr+=n;
                FD_SET(sockfd,&wset);
            }
        }
        if(FD_ISSET(sockfd,&rset)){
             if((n=read(sockfd,friptr,&fr[8192]-friptr))<0){
                if(errno!=EWOULDBLOCK){
                    printf("read error on socket!\n");
                    exit(0);
                }
            }
            else if(n==0){
                fprintf(stderr,"%s: EOF on socket\n",gf_time());
                if(stdineof){
                    return;
                }
                else{
                    printf("str_cli: server terminated prematurely\n");
                    exit(0);
                }
            }
            else{
                fprintf(stderr,"%s: read %d bytes from socket\n",gf_time(),n);
                friptr+=n;
                FD_SET(STDOUT_FILENO,&wset);
            }           
        }
        if(FD_ISSET(STDOUT_FILENO,&wset) && ((n=friptr-froptr)>0)){
            if((nwritten=write(STDOUT_FILENO,froptr,n))<0){
                if(errno!=EWOULDBLOCK){
                    printf("write error to stdout!\n");
                    exit(0);
                }
            }
            else{
                fprintf(stderr,"%s: wrote %d bytes to stdout\n",gf_time(),nwritten);
                froptr+=nwritten;
                if(froptr==friptr){
                    froptr=fr;
                    friptr=fr;
                }
            }
        }
        if(FD_ISSET(sockfd,&wset) && ((n=toiptr-tooptr)>0)){
            if((nwritten=write(sockfd,tooptr,n))<0){
                if(errno!=EWOULDBLOCK){
                    printf("write error to socket!\n");
                    exit(0);
                }
            }
            else{
                fprintf(stderr,"%s: wrote %d bytes to socket\n",gf_time(),nwritten);
                tooptr+=nwritten;
                if(tooptr==toiptr){
                    tooptr=to;
                    toiptr=to;
                    if(stdineof){
                        shutdown(sockfd,SHUT_WR);
                    }
                }
            }
        }
    }
}

int main(int argc,char** argv){
	int sockfd,SERV_PORT;
	struct sockaddr_in servaddr;
	if(argc!=3){
		printf("usage:<IPaddress> <Port\n>");
		exit(0);
	}
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
    SERV_PORT=atoi(argv[2]);
	servaddr.sin_port=htons(SERV_PORT);
	inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
	connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	str_cli(stdin,sockfd);
	exit(0);
}
