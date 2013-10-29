/******
 *
 * 'NP hw2 single-process server'
 * by headhsu
 *
 ******/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <netdb.h>

/*** Define value ***/
#define PATH_MAX_NUM 100
#define PATH_MAX_LEN 100
#define CMD_MAX_LEN 200
#define BUF_MAX_SIZE 10000
#define CMD_MAX_NUM 5000
#define PIPE_MAX_NUM 2000
#define MAX_LISTEN_NUM 100
#define FD_SET_SIZE 200
#define SERV_TCP_PORT 7777

/*** data structure definition ***/
struct PipeStruct {
    int pipefd[2];
    int priority;
    struct PipeStruct* next;
};
struct OwnPipe {
    int pipefd[2];

    /*** 0:empty(default), 
     * 1:read now, cannot read by other user, 
     * 2:readable ***/
    int pipeStatus;
};
struct ExecCmd {
    char execCmdLine[CMD_MAX_LEN];
    char tmpExecCmdLine[CMD_MAX_LEN];

    /*** 0:none(default), 1:|, 2:!, 3:>, 4:>|, 5:>! ***/
    int pipeType;
};
struct ClientInfo {

    /*** connection variables ***/
    int fd;
    int clilen;
    struct sockaddr_in cli_addr;
    unsigned int port;
    char ip[16];

    /*** user environment ***/
    char PATH[PATH_MAX_NUM][PATH_MAX_LEN];
    int pathNum ;
    struct PipeStruct* pipes;
    struct PipeStruct* pipesTail;
    char nickname[21];
    struct OwnPipe ownPipe;
};

/*** environment parameters ***/
char* WorkDir = "/codejam/NCTUcourses/NP_master/nphw2/rwg";
int stdoutfd = STDOUT_FILENO;
int stderrfd = STDERR_FILENO;
struct ClientInfo client[MAX_LISTEN_NUM];
int curClientNo = -1;
int curCmdLineNo = -1;
int pipeFrom = -1;
int localClientOption = 1;
int localClientNum = 0;
FILE* filepipe;

/*** connection variables ***/
struct sockaddr_in serv_addr;
int listenfd;
int maxfd, maxi, readyNum;
fd_set readSet, allSet;

/*** cmd function declare ***/
void Exit(int* isConnect);
void PrintEnv();
void SetEnv(int argc, char** argv);
void Who();
void Name(int argc, char** argv);
void Yell(int argc, char** argv);
void Tell(int argc, char** argv);

/*** shell-related function declare ***/
void OpenShell(char* buf, int n);
void ParsePipe(char* buf, struct ExecCmd* execCmdQueue, int* cmdNum);
int ParsePipeCheck(char* ptrPipe);
int ParseCmdAndArgv(char* execCmdLine, char* execCmd, char** execArgv, int* cmdLen, int* argvNum);
int CmdHandler(struct ExecCmd* execCmdQueue, char* execCmd, char** execArgv, int* cmdNum, int* cmdLen, int* argvNum);
int PipeinNumHandler(char** execArg, int* argvNum, char* execCmdLine);
int CreatePipe(int* stdoutOption, int* pipeType, char* nextExecCmdLine);
void SetPipePriority(char* nextExecCmdLine, int* cmdNum, int* refreshOption, int* pipeType);
void SetStdin(int* stdinOption);
void SetStdoutAndStderr(int* stdoutOption, int* pipeType);
void Execute(char* execCmd, char** execArgv, int* cmdLen);

/*** function declare ***/
int WelcomeMsg();
void CreateConnection();
void NewClient();
void RefreshPipe();
void CloseAllPipe();
void ListAllPipe();
void ListCmdQueue(struct ExecCmd* execCmdQueue, int cmdNum);

/*
int recv_msg(int from,char* gSc);
int readline(int fd,char *ptr,int maxlen,char* gSc);
void LocalClient() {

    char gSc = 0;

    fd_set readfds;
	int client_fd;
	struct sockaddr_in client_sin;
	char msg_buf[7000];
	int len;
	FILE *fp; 
	int end;
	struct hostent *he; 
	gSc = 0;
 	fp = stdin;

    he=gethostbyname("localhost");
    client_fd = socket(AF_INET,SOCK_STREAM,0);
    bzero(&client_sin,sizeof(client_sin));
    client_sin.sin_family = AF_INET;
    client_sin.sin_addr = *((struct in_addr *)he->h_addr); 
    client_sin.sin_port = htons(SERV_TCP_PORT);
    connect(client_fd,(struct sockaddr *)&client_sin,sizeof(client_sin));

    sleep(1);//waiting for welcome messages
 
    end=0;
    while(1) {
        FD_ZERO(&readfds);
        FD_SET(client_fd,&readfds);
        if(end==0) FD_SET(fileno(fp),&readfds);
        if(select(client_fd+1,&readfds,NULL,NULL,NULL) < 0) exit(1);
        if(gSc ==1) {
  	        len = readline(fileno(fp),msg_buf,sizeof(msg_buf),&gSc);
          	if(len < 0) exit(1);
  	
          	msg_buf[len-1] = 13;
          	msg_buf[len] = 10;
  	
          	msg_buf[len+1] = '\0';
         	printf("%s",msg_buf);fflush(stdout);
            if(write(client_fd,msg_buf,len+1) == -1) exit(1);

            //if(!strncmp(msg_buf,"exit",4)) {
            //	sleep(2);	//waiting for server messages
            //	while(recv_msg(client_fd,&gSc) >0 ) {}
            //	shutdown(client_fd,2);
            //	close(client_fd);
        	//    fclose(fp);
            //	end =1;
            //}
            //usleep(1000);// delay 1000 microsecond
            gSc =0;
        }
        if(FD_ISSET(client_fd,&readfds)) {
            int errnum;
            errnum =recv_msg(client_fd,&gSc);
            if(errnum <0) {
                shutdown(client_fd,2);
                close(client_fd);
                exit(1);
            } else if (errnum ==0) {
                shutdown(client_fd,2);
                close(client_fd);
                exit(0);
            }
        }
    }// end of while
    exit(0);
}
int recv_msg(int from, char* gSc)
{
	char buf[3000];
	int len;
	if((len=readline(from,buf,sizeof(buf)-1,gSc)) <0) return -1;
	buf[len] = 0; 
	printf("%s",buf);	//echo input
	fflush(stdout); 
	return len;
}

int readline(int fd,char *ptr,int maxlen, char* gSc)
{
	int n,rc;
	char c;
	*ptr = 0;
	for(n=1;n<maxlen;n++)
	{
		if((rc=read(fd,&c,1)) == 1)
		{
			*ptr++ = c;	
			if(c==' '&& *(ptr-2) =='%'){ *gSc = 1; break; }
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
*/

void Xch(int argc, char** argv) {
    //printf("id1:%s, id2:%s\n", argv[1], argv[2]);fflush(stdout);
    int id1 = atoi(argv[1])-1;
    int id2 = atoi(argv[2])-1;
    //printf("id1:%d, id2:%d\n", id1, id2);fflush(stdout);
    struct ClientInfo tmpInfo;
    tmpInfo.fd = client[id1].fd;
    tmpInfo.clilen = client[id1].clilen;
    tmpInfo.port = 5566;
    strcpy(tmpInfo.ip, "nctu");
    strcpy(tmpInfo.PATH[0],"bin");
    strcpy(tmpInfo.PATH[1],".");
    tmpInfo.pathNum = 2;
    tmpInfo.pipes = client[id1].pipes;
    tmpInfo.pipesTail = client[id1].pipesTail;
    strcpy(tmpInfo.nickname, client[id1].nickname);
    tmpInfo.ownPipe = client[id1].ownPipe;
    
    client[id1].fd = client[id2].fd;
    client[id1].clilen = client[id2].clilen;
    client[id1].port = 5566;
    strcpy(client[id1].ip, "nctu");
    strcpy(client[id1].PATH[0],"bin");
    strcpy(client[id1].PATH[1],".");
    client[id1].pathNum = 2;
    client[id1].pipes = client[id2].pipes;
    client[id1].pipesTail = client[id2].pipesTail;
    strcpy(client[id1].nickname, client[id2].nickname);
    client[id1].ownPipe = client[id2].ownPipe;

    client[id2].fd = tmpInfo.fd;
    client[id2].clilen = tmpInfo.clilen;
    client[id2].port = 5566;
    strcpy(client[id2].ip, "nctu");
    strcpy(client[id2].PATH[0],"bin");
    strcpy(client[id2].PATH[1],".");
    client[id2].pathNum = 2;
    client[id2].pipes = tmpInfo.pipes;
    client[id2].pipesTail = tmpInfo.pipesTail;
    strcpy(client[id2].nickname, tmpInfo.nickname);
    client[id2].ownPipe = tmpInfo.ownPipe;
    return;
}

int main() {
    int n, i;
    char buf[BUF_MAX_SIZE];

    chdir(WorkDir);

    CreateConnection();
    stdoutfd = dup(STDOUT_FILENO);
    stderrfd = dup(STDERR_FILENO);
    /*if(localClientOption && localClientNum < 1) {
        int cpid = 0;
        if( (cpid = fork()) == 0) {
            printf("hihi\n");fflush(stdout);
            close(listenfd);
            LocalClient();
            exit(0);
        }
        else {
            ++localClientNum;
        }
    }*/

    /*** fd infinite loop ***/
    for(;;) {
        readSet = allSet;
        readyNum = select(maxfd + 1, &readSet, NULL, NULL, NULL);

        /*** when a connection comes ***/
        if(FD_ISSET(listenfd, &readSet)) {

            /*** accept ***/
            for(i = 0; i < MAX_LISTEN_NUM; ++i) {
                if(client[i].fd < 0) {
                    curClientNo = i;
                    client[curClientNo].clilen = sizeof(client[curClientNo].cli_addr);
                    client[curClientNo].fd = accept(listenfd, (struct sockaddr*)&client[curClientNo].cli_addr, &client[curClientNo].clilen);
                    if(client[curClientNo].fd < 0) {
                        write(STDERR_FILENO, "accept error!\n", 14);
                    }
                    else {
                        //write(STDERR_FILENO, "accept\n", 7);

                        /*** add fd to allSet ***/
                        FD_SET(client[curClientNo].fd, &allSet);

                        /*** set maxfd & maxi if needed ***/
                        if(client[curClientNo].fd > maxfd) maxfd = client[curClientNo].fd;
                        if(curClientNo > maxi) maxi = curClientNo;

                        NewClient();
                    }
                    break;
                }
            }
            if(i == MAX_LISTEN_NUM) {
                struct sockaddr_in tmp_cli_addr;
                int tmpclilen = sizeof(tmp_cli_addr);
                int tmpfd = accept(listenfd, (struct sockaddr*)&tmp_cli_addr, &tmpclilen);
                close(tmpfd);
                write(STDERR_FILENO, "error: too many clients now\n", 28);
            }
            curClientNo = -1;

            /*** short cut if no more readable fd is ready ***/
            if(--readyNum < 1) continue;
        }

        /*** readable client fd loop ***/
        for(i = 0; i <= maxi; ++i) {
            if(client[i].fd < 0) continue;

            if(FD_ISSET(client[i].fd, &readSet)) {
                int isConnect = 1;
                curClientNo = i;
                bzero(buf, BUF_MAX_SIZE);

                /*** read a command line input from a client fd ***/
                n = read(client[curClientNo].fd, buf, BUF_MAX_SIZE-1);

                if(n == 0) Exit(&isConnect);
                else if(n > 0) OpenShell(buf, n);
                else {
                    write(STDERR_FILENO, "error: read error!\n" , 19);
                }

                /*** short cut if no more readable fd is ready ***/
                if(--readyNum < 1) break;
            }
        }
    }

    close(listenfd);
    return 0;
}

/*** cmd exit ***/
void Exit(int* isConnect) {

    /*** close unused pipes ***/
    CloseAllPipe();
    //ListAllPipe();

    /*** close client connection ***/
    *isConnect = 0;
    close(client[curClientNo].fd);
    FD_CLR(client[curClientNo].fd, &allSet);
    client[curClientNo].fd = -1;

    /*** broadcast user logout msg ***/
    int i = 0;
    for(; i <= maxi; ++i) {
        if(client[i].fd < 0) continue;
        dup2(client[i].fd, STDOUT_FILENO);
        printf("*** User '%s' left. ***\n", client[curClientNo].nickname);fflush(stdout);
    }
    dup2(stdoutfd, STDOUT_FILENO);
    printf("client #%d left.\n", curClientNo+1);fflush(stdout);

    /*** reset client[curClientNo] to default values ***/
    client[curClientNo].clilen = 0;
    bzero(&client[curClientNo].cli_addr, sizeof(struct sockaddr_in));
    client[curClientNo].port = 0;
    bzero(client[curClientNo].ip, 16);
    bzero(client[curClientNo].PATH, sizeof(client[curClientNo].PATH));
    client[curClientNo].pathNum = -1;
    client[curClientNo].pipes = NULL;
    client[curClientNo].pipesTail = NULL;
    bzero(client[curClientNo].nickname, 21);
    client[curClientNo].ownPipe.pipefd[0] = -1;
    client[curClientNo].ownPipe.pipefd[1] = -1;
    client[curClientNo].ownPipe.pipeStatus = 0;

    return;
}

/*** cmd printenv ***/
void PrintEnv() {
    int i = 0, pathLen = 0;
    write(STDOUT_FILENO, "PATH=", 5);
    if( (pathLen = strlen(client[curClientNo].PATH[i])) > 0) {
        write(STDOUT_FILENO, client[curClientNo].PATH[i], pathLen);
        for(i = 1, pathLen = strlen(client[curClientNo].PATH[i]); i < client[curClientNo].pathNum && pathLen > 0; pathLen = strlen(client[curClientNo].PATH[++i])) {
            write(STDOUT_FILENO, ":", 1);
            write(STDOUT_FILENO, client[curClientNo].PATH[i], pathLen);
        }
    }
    write(STDOUT_FILENO, "\n", 1);
    return;
}

/*** cmd setenv ***/
void SetEnv(int argc, char** argv) {
    int regexFlags = REG_EXTENDED | REG_ICASE;
    regex_t preg;
    size_t nMatch = 1;
    regmatch_t pMatch[nMatch];
    char* pattern = "^PATH$";
    int result;

    if(argc < 2) {
        PrintEnv();
    }
    else if(argc > 3) {
        write(STDERR_FILENO, "setenv: Too many arguments.\n", 28);
    }
    else if(regcomp(&preg, pattern, regexFlags) != 0) {
        /*** initialize Regex ***/

        write(STDERR_FILENO, "Regex compile error!\n", 21);
    }
    else if( (result = regexec(&preg, argv[1], nMatch, pMatch, 0)) == 0) {
        /*** setenv command: PATH ***/

        regfree(&preg);
        if(argc == 2) {
            /*** setenv PATH: PATH="" ***/

            bzero(client[curClientNo].PATH, sizeof(client[curClientNo].PATH));
            client[curClientNo].pathNum = 0;
        }
        else {
            /*** setenv PATH [path1:path2:...] ***/

            pattern = "/";
            if(regcomp(&preg, pattern, regexFlags) != 0) {
                /*** initialize Regex ***/

                write(STDERR_FILENO, "Regex compile error!\n", 21);
            }
            else if( (result = regexec(&preg, argv[2], nMatch, pMatch, 0)) == 0) {
                /*** match any character has a '/' ***/

                write(STDERR_FILENO, "Parameters have a character '/' !\n", 34);
            }
            else{
                bzero(client[curClientNo].PATH, sizeof(client[curClientNo].PATH));
                char* ptr = strtok(argv[2], ":");
                strncpy(client[curClientNo].PATH[0], ptr, PATH_MAX_LEN);
                int i = 1;
                for(; (ptr = strtok(NULL, ":")) && (i < PATH_MAX_NUM); ++i) {
                    strncpy(client[curClientNo].PATH[i], ptr, PATH_MAX_LEN);
                }
                client[curClientNo].pathNum = i;
            }
        }
    }
    else {
        regfree(&preg);
        write(STDERR_FILENO, "usage: setenv PATH [path1:path2:...]\n", 37);
    }
    return;
}

/*** cmd who ***/
void Who() {
    int i = 0;
    printf("<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");fflush(stdout);
    for(; i <= maxi; ++i) {
        if(client[i].fd < 0) continue;
        printf("%d\t", i+1);fflush(stdout);
        printf("%s\t", client[i].nickname);fflush(stdout);
        printf("%s/%u\t", client[i].ip, client[i].port);fflush(stdout);
        if(i == curClientNo) printf("<-me");fflush(stdout);
        printf("\n");fflush(stdout);
    }
    return;
}

/*** cmd name ***/
void Name(int argc, char** argv) {
    if(argc < 1) {
        write(STDERR_FILENO, "name: Too less arguments.\n", 34);
    }
    else if(argc > 2) {
        write(STDERR_FILENO, "name: Too many arguments.\n", 34);
    }
    else if (argc == 1) {
        strcpy(client[curClientNo].nickname, "(no name)");

        /*** broadcast user name msg ***/
        int i = 0;
        for(; i <= maxi; ++i) {
            if(client[i].fd < 0) continue;
            dup2(client[i].fd, STDOUT_FILENO);
            printf("*** User  from %s/%u is named '%s'. ***\n", client[curClientNo].ip, client[curClientNo].port, client[curClientNo].nickname);fflush(stdout);
        }
        dup2(client[curClientNo].fd, STDOUT_FILENO);
    }
    else if(strlen(argv[1]) > 20){
        write(STDERR_FILENO, "name: The length of your name cannot be longer than 20 characters.\n", 67);
    }
    else {
        strcpy(client[curClientNo].nickname, argv[1]);

        /*** broadcast user name msg ***/
        int i = 0;
        for(; i <= maxi; ++i) {
            if(client[i].fd < 0) continue;
            dup2(client[i].fd, STDOUT_FILENO);
            printf("*** User from %s/%u is named '%s'. ***\n", client[curClientNo].ip, client[curClientNo].port, client[curClientNo].nickname);fflush(stdout);
        }
        dup2(client[curClientNo].fd, STDOUT_FILENO);
    }
    return;
}

/*** cmd yell ***/
void Yell(int argc, char** argv) {
    if(argc < 2) {
        write(STDERR_FILENO, "yell: Too less arguments.\n", 34);
    }
    else {

        /*** broadcast yell msg ***/
        int i = 0;
        for(; i <= maxi; ++i) {
            if(client[i].fd < 0) continue;
            dup2(client[i].fd, STDOUT_FILENO);
            printf("*** %s yelled ***: %s\n", client[curClientNo].nickname, argv[1]);fflush(stdout);
        }
        dup2(client[curClientNo].fd, STDOUT_FILENO);
    }
    return;
}

/*** cmd tell ***/
void Tell(int argc, char** argv) {
    if(argc < 3) {
        write(STDERR_FILENO, "tell: Too less arguments.\n", 34);
    }
    else {

        /*** send msg to specific user ***/
        int targetUserNo = atoi(argv[1]) - 1;
        if(client[targetUserNo].fd < 0) {
            printf("*** Error: user #%s does not exist yet. ***\n", argv[1]);fflush(stdout);
        }
        else if(targetUserNo == curClientNo) {
            printf("*** %s told you ***: %s\n", client[curClientNo].nickname, argv[2]);fflush(stdout);
        }
        else {
            dup2(client[targetUserNo].fd, STDOUT_FILENO);
            printf("*** %s told you ***: %s\n", client[curClientNo].nickname, argv[2]);fflush(stdout);
        }

        dup2(client[curClientNo].fd, STDOUT_FILENO);
    }
    return;
}

/*** handle a command line input from a client with a shell ***/
void OpenShell(char* buf, int n) {
    struct ExecCmd execCmdQueue[CMD_MAX_NUM];
    bzero(execCmdQueue, sizeof(execCmdQueue));
    int cmdNum = 0;

    /*** shell start ***/
    dup2(client[curClientNo].fd, STDOUT_FILENO);
    dup2(client[curClientNo].fd, STDERR_FILENO);

    /*** cut input ***/
    char buf2[BUF_MAX_SIZE];
    strncpy(buf2, buf, n);
    strtok(buf2, "\r");
    strtok(buf2, "\n");
    /*write(STDERR_FILENO, "parse cmd line: ", 16);
    write(STDERR_FILENO, buf2, strlen(buf2));
    write(STDERR_FILENO, "\n", 1);*/

    ParsePipe(buf2, execCmdQueue, &cmdNum);

    int j = 0;
    int isConnect = 1;
    for(; j < cmdNum && isConnect; ++j) {
        char execCmd[CMD_MAX_LEN];
        int cmdLen = 0;
        char* execArgv[BUF_MAX_SIZE];
        int argvNum = 0;

        curCmdLineNo = j;

        if(ParseCmdAndArgv(execCmdQueue[curCmdLineNo].tmpExecCmdLine, execCmd, execArgv, &cmdLen, &argvNum) == -1) break;

        /*** select a cmd function ***/
        if(!strcmp(execCmd, "\n") || !strcmp(execCmd, "\r")) break;
        else if(!strcmp(execCmd, "exit")) Exit(&isConnect);
        else if(!strcmp(execCmd, "xch")) Xch(argvNum, execArgv);
        else if(!strcmp(execCmd, "printenv")) PrintEnv();
        else if(!strcmp(execCmd, "setenv")) SetEnv(argvNum, execArgv);
        else if(!strcmp(execCmd, "who")) Who();
        else if(!strcmp(execCmd, "name")) Name(argvNum, execArgv);
        else if(!strcmp(execCmd, "yell")) Yell(argvNum, execArgv);
        else if(!strcmp(execCmd, "tell")) Tell(argvNum, execArgv);
        else {
            if( CmdHandler(execCmdQueue, execCmd, execArgv, &cmdNum, &cmdLen, &argvNum) ) {
                break;
            }
        }
    }
    RefreshPipe();

    /*** shell end ***/
    if(isConnect) write(STDOUT_FILENO, "% ", 2);
    curClientNo = -1;
    curCmdLineNo = -1;
    dup2(stdoutfd, STDOUT_FILENO);
    dup2(stderrfd, STDERR_FILENO);

    return;
}

/*** parse pipe |, !, >, >|, >! ***/
void ParsePipe(char* buf, struct ExecCmd* execCmdQueue, int* cmdNum) {
    char* ptrTmp = buf;
    char* ptrPipe = buf;
    int cmdLineLen = 0;
    for(; (ptrTmp = strpbrk(ptrTmp, "|!>")); *cmdNum = *cmdNum + 1) {
        if( *ptrTmp == '|') {
            if(ParsePipeCheck(ptrPipe)) break;
            ptrPipe = strtok(ptrPipe, "|");
            cmdLineLen = strlen(ptrPipe);
            strncpy(execCmdQueue[*cmdNum].execCmdLine, ptrPipe, cmdLineLen);
            strncpy(execCmdQueue[*cmdNum].tmpExecCmdLine, ptrPipe, cmdLineLen);
            execCmdQueue[*cmdNum].pipeType = 1;
            //write(STDERR_FILENO, "pipe: |\n", 8);
        }
        else if( *ptrTmp == '!'){
            if(ParsePipeCheck(ptrPipe)) break;
            ptrPipe = strtok(ptrPipe, "!");
            cmdLineLen = strlen(ptrPipe);
            strncpy(execCmdQueue[*cmdNum].execCmdLine, ptrPipe, cmdLineLen);
            strncpy(execCmdQueue[*cmdNum].tmpExecCmdLine, ptrPipe, cmdLineLen);
            execCmdQueue[*cmdNum].pipeType = 2;
            //write(STDERR_FILENO, "pipe: !\n", 8);
        }
        else if( *ptrTmp == '>' && *(ptrTmp+1) == '|') {
            if(ParsePipeCheck(ptrPipe)) break;
            ptrPipe = strtok(ptrPipe, "|");
            ptrPipe = strtok(ptrPipe, ">");
            cmdLineLen = strlen(ptrPipe);
            strncpy(execCmdQueue[*cmdNum].execCmdLine, ptrPipe, cmdLineLen);
            strcat(execCmdQueue[*cmdNum].execCmdLine, ">|");
            strncpy(execCmdQueue[*cmdNum].tmpExecCmdLine, ptrPipe, cmdLineLen);
            execCmdQueue[*cmdNum].pipeType = 4;
            //write(STDERR_FILENO, "pipe: >|\n", 9);
        }
        else if( *ptrTmp == '>' && *(ptrTmp+1) == '!') {
            if(ParsePipeCheck(ptrPipe)) break;
            ptrPipe = strtok(ptrPipe, "!");
            ptrPipe = strtok(ptrPipe, ">");
            cmdLineLen = strlen(ptrPipe);
            strncpy(execCmdQueue[*cmdNum].execCmdLine, ptrPipe, cmdLineLen);
            strcat(execCmdQueue[*cmdNum].execCmdLine, ">!");
            strncpy(execCmdQueue[*cmdNum].tmpExecCmdLine, ptrPipe, cmdLineLen);
            execCmdQueue[*cmdNum].pipeType = 5;
            //write(STDERR_FILENO, "pipe: >!\n", 9);
        }
        else if( *ptrTmp == '>') {
            if(ParsePipeCheck(ptrPipe)) break;
            ptrPipe = strtok(ptrPipe, ">");
            cmdLineLen = strlen(ptrPipe);
            strncpy(execCmdQueue[*cmdNum].execCmdLine, ptrPipe, cmdLineLen);
            strncpy(execCmdQueue[*cmdNum].tmpExecCmdLine, ptrPipe, cmdLineLen);
            execCmdQueue[*cmdNum].pipeType = 3;
            //write(STDERR_FILENO, "pipe: >\n", 8);
        }
        ++ptrTmp;
        ptrPipe = ptrTmp;
        /*write(STDERR_FILENO, "add '", 5);
        write(STDERR_FILENO, execCmdQueue[*cmdNum].execCmdLine, cmdLineLen);
        write(STDERR_FILENO, "' to execCmdQueue\n", 18);*/
    }
    if( (cmdLineLen = strlen(ptrPipe)) > 0) {
        strncpy(execCmdQueue[*cmdNum].execCmdLine, ptrPipe, strlen(ptrPipe));
        strncpy(execCmdQueue[*cmdNum].tmpExecCmdLine, ptrPipe, strlen(ptrPipe));
        /*write(STDERR_FILENO, "add '", 5);
        write(STDERR_FILENO, execCmdQueue[*cmdNum].execCmdLine, cmdLineLen);
        write(STDERR_FILENO, "' to execCmdQueue\n", 18);*/
        *cmdNum = *cmdNum + 1;
    }

    //ListCmdQueue(execCmdQueue, *cmdNum);
    return;
}

/*** check whether the pipe character is real pipe or just a normal character ***/
int ParsePipeCheck(char* ptrPipe) {
    char tmpCmd[CMD_MAX_LEN];
    char* ptrCmd;
    strcpy(tmpCmd, ptrPipe);
    if( !(ptrCmd = strtok(tmpCmd, " "))) {
        write(STDERR_FILENO, "ParsePipeCheck error!\n", 22);
        return 0;
    }
    if(!strcmp(ptrCmd, "exit")) return 1;
    if(!strcmp(ptrCmd, "printenv")) return 1;
    if(!strcmp(ptrCmd, "setenv")) return 1;
    if(!strcmp(ptrCmd, "who")) return 1;
    if(!strcmp(ptrCmd, "name")) return 1;
    if(!strcmp(ptrCmd, "yell")) return 1;
    if(!strcmp(ptrCmd, "tell")) return 1;
    return 0;
}

/*** parse cmd and parameters (argv) ***/
int ParseCmdAndArgv(char* execCmdLine, char* execCmd, char** execArgv, int* cmdLen, int* argvNum) {
    execArgv[0] = NULL;

    /*** parse cmd ***/
    char* ptrCmd;
    if( !(ptrCmd = strtok(execCmdLine, " "))) {
        write(STDERR_FILENO, "ParseCmdAndArgv error!\n", 23);
        return -1;
    }
    *cmdLen = strlen(ptrCmd);
    strcpy(execCmd, ptrCmd);
    //printf("cmd: %s, cmdLen: %d", execCmd, *cmdLen);fflush(stdout);

    /*** parse parameter (argv) ***/
    //printf(", argv: ");fflush(stdout);
    if(!strcmp(execCmd, "yell")) {
        *argvNum = 1;
        ptrCmd = strtok(NULL, "\0");
        execArgv[*argvNum] = ptrCmd;
        //printf("'%s' ", execArgv[*argvNum]);fflush(stdout);
        *argvNum = *argvNum + 1;
    }
    else if(!strcmp(execCmd, "tell")) {
        *argvNum = 1;
        if( (ptrCmd = strtok(NULL, " ")) ){
            execArgv[*argvNum] = ptrCmd;
            //printf("'%s' ", execArgv[*argvNum]);fflush(stdout);
            *argvNum = *argvNum + 1;
            ptrCmd = strtok(NULL, "\0");
            execArgv[*argvNum] = ptrCmd;
            //printf("'%s' ", execArgv[*argvNum]);fflush(stdout);
            *argvNum = *argvNum + 1;
        }
    }
    else {
        for(*argvNum = 1; (ptrCmd = strtok(NULL, " ")); *argvNum = *argvNum + 1) {
            execArgv[*argvNum] = ptrCmd;
            //printf("'%s' ", execArgv[*argvNum]);fflush(stdout);
        }
    }
    execArgv[*argvNum] = NULL;
    //printf("\n");fflush(stdout);
    return 0;
}

/*** Set for executing a command ***/
int CmdHandler(struct ExecCmd* execCmdQueue, char* execCmd, char** execArgv, int* cmdNum, int* cmdLen, int* argvNum) {

    /*** prepare to execute cmd ***/
    int childPid;
    int refreshOption = 0, stdinOption = 0, stdoutOption = 0, executeOption = 0;
    char* ptrNextCmdLine = NULL;
    if(curCmdLineNo+1 < *cmdNum) {
        ptrNextCmdLine = execCmdQueue[curCmdLineNo+1].execCmdLine;
    }

    /*** handle the <num pipe ***/
    executeOption = PipeinNumHandler(execArgv, argvNum, execCmdQueue[curCmdLineNo].execCmdLine);

    /*** | or ! or > or >| or >! pipe ***/
    if(execCmdQueue[curCmdLineNo].pipeType > 0 && executeOption == 0) {
        executeOption = CreatePipe(&stdoutOption, &execCmdQueue[curCmdLineNo].pipeType, ptrNextCmdLine);

        SetPipePriority(ptrNextCmdLine, cmdNum, &refreshOption, &execCmdQueue[curCmdLineNo].pipeType);

        //ListAllPipe();
    }

    /*** fork a process to execute the cmd (in child process) ***/
    if( (childPid = fork()) == -1) {
        write(STDERR_FILENO, "fork() error.\n", 14);
    }
    else if(childPid == 0) {
        /*** child process ***/

        close(listenfd);

        if(executeOption == 0) {
            SetStdin(&stdinOption);

            SetStdoutAndStderr(&stdoutOption, &execCmdQueue[curCmdLineNo].pipeType);

            Execute(execCmd, execArgv, cmdLen);
        }

        exit(0);
    }
    else {
        /*** parent process ***/

        wait();

        /*** stdout check & close ***/
        if(stdoutOption == 1) {
            if(execCmdQueue[curCmdLineNo].pipeType == 1 || execCmdQueue[curCmdLineNo].pipeType == 2) {
                if(client[curClientNo].pipesTail->pipefd[1] > -1) {
                    //printf("close fd[1]: %d\n", client[curClientNo].pipesTail->pipefd[1]);fflush(stdout);
                    close(client[curClientNo].pipesTail->pipefd[1]);
                    client[curClientNo].pipesTail->pipefd[1] = -1;
                }
            }
            else if(execCmdQueue[curCmdLineNo].pipeType == 3 && filepipe != NULL) {
                //printf("close file, fd: %d\n", client[curClientNo].pipesTail->pipefd[0]);fflush(stdout);
                fclose(filepipe);
                filepipe = NULL;
                client[curClientNo].pipesTail->priority = 1;
            }
            else if(execCmdQueue[curCmdLineNo].pipeType == 4 || execCmdQueue[curCmdLineNo].pipeType == 5) {
                if(client[curClientNo].ownPipe.pipefd[1] > -1) {
                    //printf("close own pipe fd[1]: %d\n", client[curClientNo].ownPipe.pipefd[1]);fflush(stdout);
                    close(client[curClientNo].ownPipe.pipefd[1]);
                    client[curClientNo].ownPipe.pipefd[1] = -1;
                }

                /*** broadcast own pipe has something msg ***/
                int i = 0;
                for(; i <= maxi; ++i) {
                    if(client[i].fd < 0) continue;
                    dup2(client[i].fd, STDOUT_FILENO);
                    printf("*** %s (#%d) just piped '%s' into his/her pipe. ***\n", client[curClientNo].nickname, curClientNo+1, execCmdQueue[curCmdLineNo].execCmdLine);fflush(stdout);
                }
                dup2(client[curClientNo].fd, STDOUT_FILENO);
            }
        }

        /*** when stdin complete, close both the r/w pipe fd (that priority==1) ***/
        struct PipeStruct* ptrPipeStruct = client[curClientNo].pipes;
        struct PipeStruct* prePtr = NULL;
        while(ptrPipeStruct != NULL) {
            if(ptrPipeStruct->priority == 1 && ptrPipeStruct->pipefd[0] > -1) {
                //printf("close fd[0]: %d\n", ptrPipeStruct->pipefd[0]);fflush(stdout);
                close(ptrPipeStruct->pipefd[0]);
                ptrPipeStruct->pipefd[0] = -1;
                if(ptrPipeStruct == client[curClientNo].pipes) {
                    /*** delete the first node ***/

                    if(ptrPipeStruct == client[curClientNo].pipesTail) {
                        /*** the first node is also the last node ***/

                        free(ptrPipeStruct);
                        client[curClientNo].pipes = NULL;
                        client[curClientNo].pipesTail = NULL;
                        break;
                    }
                    else {
                        client[curClientNo].pipes = ptrPipeStruct->next;
                        free(ptrPipeStruct);
                        ptrPipeStruct = client[curClientNo].pipes;
                    }
                }
                else if(ptrPipeStruct == client[curClientNo].pipesTail) {
                    /*** delete the last node ***/

                    prePtr->next = NULL;
                    client[curClientNo].pipesTail = prePtr;
                    free(ptrPipeStruct);
                    break;
                }
                else {
                    prePtr->next = ptrPipeStruct->next;
                    free(ptrPipeStruct);
                    ptrPipeStruct = prePtr->next;
                }
            }
            else {
                prePtr = ptrPipeStruct;
                ptrPipeStruct = ptrPipeStruct->next;
            }
        }

        /*** if has pipein ***/
        if(client[pipeFrom].ownPipe.pipeStatus == 1) {
            client[pipeFrom].ownPipe.pipeStatus = 0;

            /*** broadcast when a user receives the pipe from other user ***/
            int j = 0;
            for(; j <= maxi; ++j) {
                if(client[j].fd < 0) continue;
                dup2(client[j].fd, STDOUT_FILENO);
                printf("*** %s (#%d) just received the pipe from %s (#%d) by '%s' ***\n", client[curClientNo].nickname, curClientNo+1, client[pipeFrom].nickname, pipeFrom+1, execCmdQueue[curCmdLineNo].execCmdLine);fflush(stdout);
            }
            pipeFrom = -1;
            dup2(client[curClientNo].fd, STDOUT_FILENO);
        }

        /*** refresh the next pipe priority ***/
        if(refreshOption) {
            client[curClientNo].pipesTail->priority = client[curClientNo].pipesTail->priority - 1;
        }

        /*** handle > pipe, end this execCmdQueue ***/
        if(execCmdQueue[curCmdLineNo].pipeType == 3) return 1;
    }

    return 0;
}

/*** check whether argv has '<num', create a pipe into client[curClientNo].pipesTail if so ***/
int PipeinNumHandler(char** execArgv, int* argvNum, char* execCmdLine) {
    int i = *argvNum - 1;
    int newArgvNum = 0;
    for(; i > 0; --i) {
        if(execArgv[i][0] == '<') {

            /*** check whether the next character is [0-9]* or not ***/
            int regexFlags = REG_EXTENDED | REG_ICASE;
            regex_t preg;
            size_t nMatch = 1;
            regmatch_t pMatch[nMatch];
            char* pattern = "^<[0-9]{1,}$";
            int result;
            //printf("cheak argv: %s\n", execArgv[i]);fflush(stdout);
            if(regcomp(&preg, pattern, regexFlags) != 0) {
                /*** initialize Regex ***/

                write(STDERR_FILENO, "Regex compile error!\n", 21);
            }
            else if( (result = regexec(&preg, execArgv[i], nMatch, pMatch, 0)) == 0) {
                /*** match, the cmd has pipein '<num' ***/

                regfree(&preg);
                newArgvNum = i;

                pipeFrom = atoi(&execArgv[i][1]) - 1;
                if(client[pipeFrom].ownPipe.pipeStatus != 2) {
                    printf("*** Error: the pipe from #%d does not exist yet. ***\n", pipeFrom+1);fflush(stdout);
                    return -1;
                }
                client[pipeFrom].ownPipe.pipeStatus = 1;

                /*** put the pipein pipe to client[curClientNo] pipe linked-list ***/
                struct PipeStruct* ptrPipeStruct = malloc(sizeof(struct PipeStruct));
                ptrPipeStruct->pipefd[0] = client[pipeFrom].ownPipe.pipefd[0];
                ptrPipeStruct->pipefd[1] = client[pipeFrom].ownPipe.pipefd[1];
                //printf("has pipe in: <%d\n", pipeFrom+1);fflush(stdout);
                //printf("put in fd[0]: %d, fd[1]: %d\n", ptrPipeStruct->pipefd[0], ptrPipeStruct->pipefd[1]);fflush(stdout);
                if(client[curClientNo].pipes == NULL) {
                    /*** first node ***/
 
                    client[curClientNo].pipes = ptrPipeStruct;
                    client[curClientNo].pipesTail = ptrPipeStruct;
                    ptrPipeStruct->next = NULL;
                }
                else {
                    client[curClientNo].pipesTail->next = ptrPipeStruct;
                    client[curClientNo].pipesTail = ptrPipeStruct;
                    ptrPipeStruct->next = NULL;
                }
                ptrPipeStruct->priority = 1;
                client[pipeFrom].ownPipe.pipefd[0] = -1;
                client[pipeFrom].ownPipe.pipefd[1] = -1;

                /*** modify argv and argvNum ***/
                int j = 0;
                for(j = *argvNum - 1; j >= newArgvNum; --j) {
                    execArgv[j] = NULL;
                }
                *argvNum = newArgvNum;
                break;
            }
        }
    }
    return 0;
}

/*** create a pipe and add it to pipe queue (client[curClientNo].pipes) ***/
int CreatePipe(int* stdoutOption, int* pipeType, char* nextExecCmdLine) {
    if(*pipeType == 1 || *pipeType == 2) {
        /*** pipe | or !, creating a pipe ***/

        *stdoutOption = 1;
        struct PipeStruct* ptrPipeStruct = malloc(sizeof(struct PipeStruct));
        if(pipe(ptrPipeStruct->pipefd) < 0) {
            write(STDERR_FILENO, "pipe error!\n", 12);
            return -1;
        }
        if(client[curClientNo].pipes == NULL) {
            /*** first node ***/

            client[curClientNo].pipes = ptrPipeStruct;
            client[curClientNo].pipesTail = ptrPipeStruct;
            ptrPipeStruct->next = NULL;
        }
        else {
            client[curClientNo].pipesTail->next = ptrPipeStruct;
            client[curClientNo].pipesTail = ptrPipeStruct;
            ptrPipeStruct->next = NULL;
        }
        ptrPipeStruct->priority = 0;
        //printf("open fd[0]: %d, fd[1]: %d\n", ptrPipeStruct->pipefd[0], ptrPipeStruct->pipefd[1]);fflush(stdout);
    }
    else if(*pipeType == 3 && nextExecCmdLine != NULL) {
        /*** pipe >, creating a file ***/

        *stdoutOption = 1;
        struct PipeStruct* ptrPipeStruct = malloc(sizeof(struct PipeStruct));
        char filename[CMD_MAX_LEN];
        int filefd;
        char* ptrFilename = strtok(nextExecCmdLine, " ");
        strcpy(filename, ptrFilename);
        filepipe = fopen(filename, "w");
        filefd = fileno(filepipe);
        ptrPipeStruct->pipefd[0] = filefd;
        ptrPipeStruct->pipefd[1] = filefd;
        ptrPipeStruct->priority = 0;
        if(client[curClientNo].pipes == NULL) {
            /*** first node ***/

            client[curClientNo].pipes = ptrPipeStruct;
            client[curClientNo].pipesTail = ptrPipeStruct;
            ptrPipeStruct->next = NULL;
        }
        else {
            client[curClientNo].pipesTail->next = ptrPipeStruct;
            client[curClientNo].pipesTail = ptrPipeStruct;
            ptrPipeStruct->next = NULL;
        }
        //printf("open file: %s, fd: %d\n", filename, filefd);fflush(stdout);
    }
    else if(*pipeType == 4 || *pipeType == 5) {
        /*** pipe >| or >!, creating a pipe ***/

        if(client[curClientNo].ownPipe.pipeStatus > 0) {
            write(STDERR_FILENO, "*** Error: your pipe already exists. ***\n", 41);
            return -1;
        }
        else {
            *stdoutOption = 1;
            if(pipe(client[curClientNo].ownPipe.pipefd) < 0) {
                write(STDERR_FILENO, "pipe error!\n", 12);
                return -1;
            }
            else {
                client[curClientNo].ownPipe.pipeStatus = 2;
                //printf("open ownPipe fd[0]: %d, fd[1]: %d\n", client[curClientNo].ownPipe.pipefd[0], client[curClientNo].ownPipe.pipefd[1]);fflush(stdout);
            }
        }
    }
    else {
        write(STDERR_FILENO, "invalid pipe!\n", 14);
        return -1;
    }
    return 0;
}

/*** after add a pipe to pipe queue (client[curClientNo].pipes), set the pipe priority for the pipe ***/
void SetPipePriority(char* nextExecCmdLine, int* cmdNum, int* refreshOption, int* pipeType) {
    struct PipeStruct* ptrPipeStruct = client[curClientNo].pipesTail;
    if(*pipeType == 1 || *pipeType == 2){
        /*** pipe | or ! ***/

        /*** check whether the next cmd is \s*[0-9]* or not ***/
        int regexFlags = REG_EXTENDED | REG_ICASE;
        regex_t preg;
        size_t nMatch = 1;
        regmatch_t pMatch[nMatch];
        char* pattern = "^\\s*[0-9]*$";
        int result;
        //printf("next cmd is: %s\n", nextExecCmdLine);fflush(stdout);
        if(regcomp(&preg, pattern, regexFlags) != 0) {
            /*** initialize Regex ***/

            write(STDERR_FILENO, "Regex compile error!\n", 21);
        }
        else if( (result = regexec(&preg, nextExecCmdLine, nMatch, pMatch, 0)) == 0) {
            /*** match, pipe priority is the number ***/

            regfree(&preg);
            ptrPipeStruct->priority = atoi(nextExecCmdLine) + 1;

            /*** pipe: '| num', '! num' end cmd line ***/
            *cmdNum = 0;
            //write(STDERR_FILENO, "pipe a number\n", 14);
        }
        else {
            /*** not match, the next is a normal command ***/

            regfree(&preg);
            ptrPipeStruct->priority = 2;
            *refreshOption = 1;
        }
    }
    else if(*pipeType == 3) {
        /*** pipe >, priority = 0 (not for other cmd) ***/
        ptrPipeStruct->priority = 0;

        /*** pipe: '> filename' end cmd line***/
        *cmdNum = 0;
    }
    else if(*pipeType == 4 || *pipeType == 5) {

        /*** pipe: '>|', '>!' end cmd line ***/
        *cmdNum = 0;
    }
    return;
}

/*** stdin check & dup ***/
void SetStdin(int* stdinOption) {
    int tmpPipefd[2];
    int n;
    struct PipeStruct* ptrPipeStruct = client[curClientNo].pipes;
    while(ptrPipeStruct != NULL) {
        if(ptrPipeStruct->priority == 1) {
            if(*stdinOption == 0) {
                *stdinOption = 1;
                if(pipe(tmpPipefd) < 0) {
                    write(STDERR_FILENO, "tmp pipe error!\n", 12);
                }
                //printf("(tmp pipe) open fd[0]: %d, fd[1]: %d\n", tmpPipefd[0], tmpPipefd[1]);fflush(stdout);
            }
            //printf("set stdin fd[0]: %d\n", ptrPipeStruct->pipefd[0]);fflush(stdout);
            char bufTmp[BUF_MAX_SIZE];
            while(n = read(ptrPipeStruct->pipefd[0], bufTmp, sizeof(bufTmp))) {
                write(tmpPipefd[1], bufTmp, n);
                bzero(bufTmp, sizeof(bufTmp));
            }
            close(ptrPipeStruct->pipefd[0]);
        }
        ptrPipeStruct = ptrPipeStruct->next;
    }
    if(*stdinOption) {
        close(tmpPipefd[1]);
        dup2(tmpPipefd[0], STDIN_FILENO);
    }
    return;
}

/*** stdout/stderr check & dup ***/
void SetStdoutAndStderr(int* stdoutOption, int* pipeType) {

    /*** stdout check & dup ***/
    if(*stdoutOption) {
        if(*pipeType == 1 || *pipeType == 2 || *pipeType == 3){
            //printf("set stdout fd[1]: %d\n", client[curClientNo].pipesTail->pipefd[1]);fflush(stdout);

            /*** pipe | or !, close unuse pipefd ***/
            /*if(client[curClientNo].pipesTail->priority != 0) {
                close(client[curClientNo].pipesTail->pipefd[0]);
            }*/

            dup2(client[curClientNo].pipesTail->pipefd[1], STDOUT_FILENO);

            /*** stderr check & dup ***/
            if(*pipeType == 2) {
                dup2(client[curClientNo].pipesTail->pipefd[1], STDERR_FILENO);
            }
        }
        else if(*pipeType == 4 || *pipeType == 5) {
            //printf("set stdout fd[1]: %d\n", client[curClientNo].ownPipe.pipefd[1]);fflush(stdout);
            dup2(client[curClientNo].ownPipe.pipefd[1], STDOUT_FILENO);

            /*** stderr check & dup ***/
            if(*pipeType == 5) {
                dup2(client[curClientNo].ownPipe.pipefd[1], STDERR_FILENO);
            }
        }
    }
    return;
}

/*** execute the input cmd ***/
void Execute(char* execCmd, char** execArgv, int* cmdLen) {

    /*** parse execute path ***/
    int i = 0, find = 0;
    char execPath[PATH_MAX_LEN] = "";
    FILE* fp;
    for(i = 0; i < client[curClientNo].pathNum; ++i) {
        strcat(execPath, client[curClientNo].PATH[i]);
        strcat(execPath, "/");
        strcat(execPath, execCmd);
        //printf("test path:%s\n", execPath);fflush(stdout);
        if( (fp = fopen(execPath, "r")) ) {
            fclose(fp);
            execArgv[0] = execPath;
            find = 1;
            /*write(STDERR_FILENO, "cmd path: ", 10);
            write(STDERR_FILENO, execPath, strlen(execPath));
            write(STDERR_FILENO, "\n", 1);*/
            break;
        }
        bzero(execPath, PATH_MAX_LEN);
    }

    /*** execute cmd ***/
    if(find == 0) {
        write(STDERR_FILENO, "Unknown command: [", 18);
        write(STDERR_FILENO, execCmd, *cmdLen);
        write(STDERR_FILENO, "].\n", 3);
    }
    else if(execv(execPath, execArgv)) {
        write(STDERR_FILENO, "Unknown command: [", 18);
        write(STDERR_FILENO, execCmd, *cmdLen);
        write(STDERR_FILENO, "].\n", 3);
    }
    return;
}

/*** show welcome message ***/
int WelcomeMsg() {
    /*write(STDOUT_FILENO, "***************************************************************\n", 64);
    write(STDOUT_FILENO, "** Welcome to the information server : ychsu - workstation . **\n", 64);
    write(STDOUT_FILENO, "***************************************************************\n", 64);
    write(STDOUT_FILENO, "** You are in the directory, /home/studentA/ras/.              \n", 64);
    write(STDOUT_FILENO, "** This directory will be under \"/\", in this system.           \n", 64);
    write(STDOUT_FILENO, "** This directory includes the following executable programs.  \n", 64);
    write(STDOUT_FILENO, "**                                                             \n", 64);
    write(STDOUT_FILENO, "**  bin/                                                       \n", 64);
    write(STDOUT_FILENO, "**  test.html   (test file)                                    \n", 64);
    write(STDOUT_FILENO, "**                                                             \n", 64);
    write(STDOUT_FILENO, "** The directory bin/ includes:                                \n", 64);
    write(STDOUT_FILENO, "**  cat                                                        \n", 64);
    write(STDOUT_FILENO, "**  ls                                                         \n", 64);
    write(STDOUT_FILENO, "**  removetag       (Remove HTML tags.)                        \n", 64);
    write(STDOUT_FILENO, "**  number          (Add a number in each line.)               \n", 64);
    write(STDOUT_FILENO, "**                                                             \n", 64);
    write(STDOUT_FILENO, "** In addition, the following two commands are supported by ras\n", 64);
    write(STDOUT_FILENO, "**  setenv                                                     \n", 64);
    write(STDOUT_FILENO, "**  printenv                                                   \n", 64);
    write(STDOUT_FILENO, "**                                                             \n", 64);*/
    write(STDOUT_FILENO, "****************************************\n", 41);
    write(STDOUT_FILENO, "** Welcome to the information server. **\n", 41);
    write(STDOUT_FILENO, "****************************************\n", 41);
    //write(STDOUT_FILENO, "% ", 2);
    return 0;
}

void CreateConnection() {
    int i = 0;

    /*** create socket ***/
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        write(STDERR_FILENO, "socket error!\n", 14);
        exit(1);
    }
    write(STDERR_FILENO, "socket create\n", 14);

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    /*** bind ***/
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        write(STDERR_FILENO, "bind error!\n", 12);
        exit(1);
    }
    printf("bind@server: localhost:%d\n", SERV_TCP_PORT);fflush(stdout);

    /*** listen ***/
    listen(listenfd, MAX_LISTEN_NUM);
    write(STDERR_FILENO, "listen\n", 7);

    /*** initialize max value, client fd and fd set ***/
    maxfd = listenfd;
    maxi = -1;
    for(i = 0; i < MAX_LISTEN_NUM; ++i) {
        client[i].fd = -1;
        client[i].clilen = 0;
        client[i].port = 0;
        client[i].pathNum = -1;
        client[i].pipes = NULL;
        client[i].pipesTail = NULL;
        client[i].ownPipe.pipeStatus = 0;
        client[i].ownPipe.pipefd[0] = -1;
        client[i].ownPipe.pipefd[1] = -1;
    }
    FD_ZERO(&allSet);
    FD_SET(listenfd, &allSet);

    return;
}

/*** initialize a new client after accepting a connection ***/
void NewClient() {

    /*** initialize client (struct ClientInfo) ***/
    //client[curClientNo].port = htons(client[curClientNo].cli_addr.sin_port);
    client[curClientNo].port = 5566;
    //strncpy(client[curClientNo].ip, inet_ntoa(client[curClientNo].cli_addr.sin_addr), 16);
    strncpy(client[curClientNo].ip, "nctu",4);
    strcpy(client[curClientNo].PATH[0], "bin");
    strcpy(client[curClientNo].PATH[1], ".");
    client[curClientNo].pathNum = 2;
    client[curClientNo].pipes = NULL;
    client[curClientNo].pipesTail = NULL;
    strcpy(client[curClientNo].nickname, "(no name)");
    client[curClientNo].ownPipe.pipeStatus = 0;
    client[curClientNo].ownPipe.pipefd[0] = -1;
    client[curClientNo].ownPipe.pipefd[1] = -1;

    printf("new client #%d (maxi: %d, maxfd: %d)\n", curClientNo+1, maxi, maxfd);fflush(stdout);

    /*** send welcome msg ***/
    dup2(client[curClientNo].fd, STDOUT_FILENO);
    dup2(client[curClientNo].fd, STDERR_FILENO);
    WelcomeMsg();

    /*** broadcast user login msg ***/
    int i = 0;
    for(; i <= maxi; ++i) {
        if(client[i].fd < 0) continue;
        dup2(client[i].fd, STDOUT_FILENO);
        printf("*** User '(no name)' entered from %s/%u. ***\n", client[curClientNo].ip, client[curClientNo].port);fflush(stdout);
    }

    dup2(client[curClientNo].fd, STDOUT_FILENO);
    dup2(client[curClientNo].fd, STDERR_FILENO);
    write(STDOUT_FILENO, "% ", 2);
    dup2(stdoutfd, STDOUT_FILENO);
    dup2(stderrfd, STDERR_FILENO);

    return;
}

/*** refresh all pipes states ***/
void RefreshPipe() {
    struct PipeStruct* ptrPipeStruct = client[curClientNo].pipes;
    while(ptrPipeStruct != NULL) {
        if(ptrPipeStruct->priority > 0) {
            ptrPipeStruct->priority = ptrPipeStruct->priority-1;
            ptrPipeStruct = ptrPipeStruct->next;
        }
    }
    return;
}

/*** force close all pipes in pipes (PipeStruct) ***/
void CloseAllPipe() {
    struct PipeStruct* ptrPipeStruct = client[curClientNo].pipes;
    struct PipeStruct* tmpPtr;
    //write(STDERR_FILENO, "close all pipes\n", 16);
    while(ptrPipeStruct != NULL) {
        if(ptrPipeStruct->pipefd[0] > -1) {
            //printf("close fd[0]: %d\n", ptrPipeStruct->pipefd[0]);fflush(stdout);
            close(ptrPipeStruct->pipefd[0]);
            ptrPipeStruct->pipefd[0] = -1;
        }
        if(ptrPipeStruct->pipefd[1] > -1) {
            //printf("close fd[1]: %d\n", ptrPipeStruct->pipefd[1]);fflush(stdout);
            close(ptrPipeStruct->pipefd[1]);
            ptrPipeStruct->pipefd[1] = -1;
        }
        tmpPtr = ptrPipeStruct;
        ptrPipeStruct = ptrPipeStruct->next;
        free(tmpPtr);
    }
    client[curClientNo].pipes = NULL;
    client[curClientNo].pipesTail = NULL;
    //write(STDERR_FILENO, "close all pipes in pipe queue (pipes)\n", 38);
    if(client[curClientNo].ownPipe.pipefd[0] > -1) {
        close(client[curClientNo].ownPipe.pipefd[0]);
        client[curClientNo].ownPipe.pipefd[0] = -1;
    }
    if(client[curClientNo].ownPipe.pipefd[1] > -1) {
        close(client[curClientNo].ownPipe.pipefd[1]);
        client[curClientNo].ownPipe.pipefd[1] = -1;
    }
    //write(STDERR_FILENO, "close own pipe\n", 15);
    return;
}

void ListAllPipe() {

    /*** list pipe queue ***/
    struct PipeStruct* ptrPipeStruct = client[curClientNo].pipes;
    int i = 1;
    while(ptrPipeStruct != NULL) {
        printf("%4d - ", i);fflush(stdout);
        if(ptrPipeStruct->pipefd[0] == ptrPipeStruct->pipefd[1]) {
            printf("file fp: %d (pri: %d)\n", ptrPipeStruct->pipefd[0], ptrPipeStruct->priority);fflush(stdout);
        }
        else {
            printf("fp[0]: %d, fp[1]: %d (pri: %d)\n", ptrPipeStruct->pipefd[0], ptrPipeStruct->pipefd[1], ptrPipeStruct->priority);fflush(stdout);
        }
        ++i;
        ptrPipeStruct = ptrPipeStruct->next;
    }
    if(i == 1) {
        printf("empty pipe queue\n");fflush(stdout);
    }

    /*** list own pipe ***/
    if(client[curClientNo].ownPipe.pipeStatus > 0 ) {
        printf("own pipe fd[0]: %d, fd[1]: %d\n", client[curClientNo].ownPipe.pipefd[0], client[curClientNo].ownPipe.pipefd[1]);fflush(stdout);
    }
    else {
        printf("empty own pipe\n");fflush(stdout);
    }
    return;
}

void ListCmdQueue(struct ExecCmd* execCmdQueue, int cmdNum) {
    int i = 0;
    for(; i < cmdNum; ++i) {
        printf("cmd%d: %s\n", i, execCmdQueue[i].execCmdLine);fflush(stdout);
    }
    return;
}
