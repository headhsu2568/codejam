#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>

#define MAXUSER 500

#define EXIT_STR "exit\r\n"


int recv_msg(int userno,int from)
{
    char buf[3000],*tmp;
    int len,i;
    if((len=read(from,buf,sizeof(buf)-1)) <0) 
        return -1;
        
    buf[len] = 0;
    if(len>0)
    {
        for(tmp=strtok(buf,"\n");tmp;tmp=strtok(NULL,"\n"))
        {
            //printf("\n---------------user %d---------------\n",userno); 
            printf("%d| %s\n",userno,tmp);	//echo input
        }
    }
    fflush(stdout); 
    return len;
}

int readline(int fd,char *ptr,int maxlen)
{
        int n,rc;
	char c;
        *ptr = 0;
        for(n=1;n<maxlen;n++)
        {
        	if((rc=read(fd,&c,1)) == 1)
        	{
        		*ptr++ = c;
        		if(c=='\n')  break;
        	}
        	else if(rc==0)
        	{
        		if(n==1)     return(0);
        		else         break;
        	}
        	else
        		return(-1);
        }
        return(n);
}      
  

int main(int argc,char *argv[])
{
    fd_set readfds;
    fd_set afds;
    int    client_fd[MAXUSER];
    struct sockaddr_in client_sin[MAXUSER];
    char buf[3000];
    char msg_buf[3000],msg_buf1[3000];
    int len;
    int SERVER_PORT;
    FILE *fd; 
    int i,end;
    struct hostent *he; 
 
    if(argc == 3)
        fd = stdin;
    else if(argc == 4)
        fd = fopen(argv[3], "r");
    else
    {
        fprintf(stderr,"Usage : client <server ip> <port> <testfile>\n");
        exit(1);
    }    

    if((he=gethostbyname(argv[1])) == NULL)
    {
        fprintf(stderr,"Usage : client <server ip> <port> <testfile>");
        exit(1);
    }
                              
    SERVER_PORT = (u_short)atoi(argv[2]);
 
    for(i=0;i<MAXUSER;i++)
    {
        client_fd[i] = socket(AF_INET,SOCK_STREAM,0);
        bzero(&client_sin[i],sizeof(client_sin[i]));
        client_sin[i].sin_family = AF_INET;
        client_sin[i].sin_addr = *((struct in_addr *)he->h_addr); 
        client_sin[i].sin_port = htons(SERVER_PORT);
    }

    //end=0;// del by sapp
    ////////////////// add by sapp
    
    FD_ZERO(&readfds);
    FD_ZERO(&afds);
    FD_SET(fileno(fd),&afds);//設檔案
    ///////////////// end by sapp
    while(1)
    { 
        memcpy(&readfds,&afds,sizeof(fd_set));// add by sapp
        //FD_ZERO(&readfds);    //del by sapp
        /*for(i=0;i<MAXUSER;i++)//del by sapp
        {
            FD_SET(client_fd[i],&readfds);
        }*/
        //if(end==0)        //del by sapp
          //  FD_SET(fileno(fd),&readfds);  //del by sapp
        if(select(MAXUSER+5,&readfds,NULL,NULL,NULL) < 0)
            return 0 ;
        for(i=0;i<MAXUSER;i++)
        {  
            if(FD_ISSET(client_fd[i],&readfds))
            {//接收server送來的訊息
                //接收message
                if(recv_msg(i,client_fd[i]) <0)
                {
                    shutdown(client_fd[i],2);
                    close(client_fd[i]);
                    exit(1);
                }
            }
        }
  
        if(FD_ISSET(fileno(fd),&readfds))
        {//從檔案
            //送meesage
            len = readline(fileno(fd),msg_buf,sizeof(msg_buf));
            if(len < 0) 
                exit(1);
            
            msg_buf[len-1] = 13;
            msg_buf[len] = 10;
            
            msg_buf[len+1] = '\0';
            fflush(stdout);
            if(!strncmp(msg_buf,"exit",4))
            {//全部離開
                printf("\n%s",msg_buf);
                sleep(2);	//waiting for server messages
                for(i=0;i<MAXUSER;i++)
                {
                    if (FD_ISSET(client_fd[i],&afds))//modify by sapp
                    {
                        if(write(client_fd[i],EXIT_STR ,6) == -1) return -1;
                        while(recv_msg(i,client_fd[i]) >0);
                        shutdown(client_fd[i],2);
                        close(client_fd[i]);
                        FD_CLR(client_fd[i], &afds);
                    }
                }
                FD_CLR(fileno(fd), &afds);
                fclose(fd);
                exit(0);
                //end =1;//del by sapp
            }
            else if(!strncmp(msg_buf,"login",5))
            {//登入
                printf("\n%s",msg_buf);
                //i=msg_buf[5]-'0'; //del by fou                
                sscanf(msg_buf, "login%d", &i); // modify by fou
                if(i<MAXUSER&&i>=0)
                {
                    if(connect(client_fd[i],(struct sockaddr *)&client_sin[i],sizeof(client_sin[i])) == -1)
                    {
                        perror("");
                        printf("connect fail\n");
                    }
                    else
                        FD_SET(client_fd[i],&afds);
                }
            }
            else if (!strncmp(msg_buf,"logout",6)) //add by fou
            {//登出
                printf("\n%s",msg_buf);
                sscanf(msg_buf, "logout%d", &i);
                if(i<MAXUSER&&i>=0)
                {
                    if (FD_ISSET(client_fd[i],&afds))//modify by sapp
                    {
                        if(write(client_fd[i], EXIT_STR,6) == -1) return -1;
                        while(recv_msg(i,client_fd[i]) >0 );
                        shutdown(client_fd[i],2);
                        close(client_fd[i]);
                        FD_CLR(client_fd[i], &afds);
                        FD_CLR(client_fd[i],&readfds);
                        client_fd[i] = socket(AF_INET,SOCK_STREAM,0);
                    }
                }
            }  
            else
            {//送cmd
                char tmpArr[20];
                // i=msg_buf[0]-'0'; //del by fou
                sscanf(msg_buf, "%d", &i);//modify by fou                
                sprintf(tmpArr, "%d", i);         
               	strcpy(msg_buf1,&msg_buf[strlen(tmpArr)+1]);
                //	strcpy(msg_buf1,&msg_buf[2]); 
                printf("\n%d %% %s",i,msg_buf1);
                if(i<MAXUSER){
                		//write(client_fd[i],msg_buf1,len-1,0);
                    write(client_fd[i],msg_buf1,strlen(msg_buf1));
                }
            }   
            usleep(300000);       //sleep 1 sec before next commandlready at oldest change 
        }

    } // end of while
return 0 ;
}  // end of main

