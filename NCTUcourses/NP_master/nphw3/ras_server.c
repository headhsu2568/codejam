/******
 *
 * 'NP hw3 ras server (modify the hw1 ras server)'
 * by headhsu
 *
 * gcc -fno-stack-protector -o ras_server ras_server.c
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

/*** Define value ***/
#define PATH_MAX_NUM 100
#define PATH_MAX_LEN 100
#define CMD_MAX_LEN 200
//#define BUF_MAX_SIZE 20480
#define CMD_MAX_NUM 10000
#define PIPE_MAX_NUM 2000
#define SERV_TCP_PORT 7771

/*** data structure definition ***/
struct PipeStruct {
    int pipefd[2];
    int priority;
    struct PipeStruct* next;
};
struct ExecCmd {
    char execCmdLine[CMD_MAX_LEN];

    /*** 0:none(default), 1:|, 2:!, 3:>, 4&5:(only used in rwg server), 6:<<EOF ***/
    int pipeType;
};

/*** environment parameters ***/
int BUF_MAX_SIZE = 40960;
char* WorkDir = "./ras";
char PATH[PATH_MAX_NUM][PATH_MAX_LEN] = {"bin", "."};
int pathNum = 2;
int welcomeMsgOption = 1;
struct PipeStruct* pipes;
struct PipeStruct* pipesTail;
int remoteOption = 1;
int connectionOption = 0;
int stdoutfd = STDOUT_FILENO;
int stderrfd = STDERR_FILENO;
FILE* filepipe;

/*** connection variables ***/
int sockfd, newsockfd, clilen;
struct sockaddr_in cli_addr, serv_addr;

/*** function delcare ***/
int WelcomeMsg();
void CreateConnection();
void ReadCmdLine(char** buf, int* n);
int ReadCmdLineFromRemote(char** buf, int* n);
void ParsePipe(char* buf, struct ExecCmd* execCmdQueue, int* cmdNum);
int ParseCmdAndArgv(char* execCmdLine, char* execCmd, char** execArgv, int* cmdLen, int* argvNum, int* pipeType);
void CreatePipe(int* stdoutOption, int* pipeType, char* nextExecCmdLine);
void SetPipePriority(char* nextExecCmdLine, int* cmdNum, int* refreshOption, int* pipeType);
void SetStdin(int* stdinOption);
void SetStdoutAndStderr(int* stdoutOption, int* pipeType);
void Execute(char* execCmd, char* execPath, char** execArgv, int* cmdLen);
void PrintEnv();
void SetEnv(int argc, char** argv);
void RefreshPipe();
void CloseAllPipe();
void ListAllPipe();

int main() {
    int n, childPid;
    //char buf[BUF_MAX_SIZE];
    //char buf2[BUF_MAX_SIZE];
    char* buf = malloc(BUF_MAX_SIZE);
    char execPath[PATH_MAX_LEN];
    char execCmd[CMD_MAX_LEN];
    int cmdLen = 0;
    char* execArgv[BUF_MAX_SIZE];
    int argvNum = 0;
    struct ExecCmd execCmdQueue[CMD_MAX_NUM];
    int cmdNum = 0;

    chdir(WorkDir);

    if(remoteOption) CreateConnection();
    stdoutfd = dup(STDOUT_FILENO);
    stderrfd = dup(STDERR_FILENO);

    /*** a connection come ***/
    for(;;) {

        /*** accept ***/
        if(remoteOption) {
            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
            if(newsockfd < 0) {
                write(STDERR_FILENO, "accept error!\n", 14);
                exit(1);
            }
            //write(STDERR_FILENO, "accept\n", 7);
            printf("new client fd: %d\n", newsockfd);fflush(stdout);
    
            /*** for remote client, stdout/stderr is client socket ***/
            if(remoteOption){ 
                dup2(newsockfd, STDOUT_FILENO);
                dup2(newsockfd, STDERR_FILENO);
            }
        }
        connectionOption = 1;
        //int line = 1;

        /*** shell start ***/
        while(connectionOption) {
            if(welcomeMsgOption) {
                welcomeMsgOption = WelcomeMsg();
            }
            //printf("%4d", line++);fflush(stdout);
            write(STDOUT_FILENO, "% ", 2);

            /*** clean & initialize variables ***/
            bzero(buf, BUF_MAX_SIZE);
            bzero(execPath, PATH_MAX_LEN);
            bzero(execCmd, CMD_MAX_LEN);
            bzero(execArgv, sizeof(execArgv));
            bzero(execCmdQueue, sizeof(execCmdQueue));
            cmdNum = 0;
            argvNum = 0;
            cmdLen = 0;

            if(remoteOption) connectionOption = ReadCmdLineFromRemote(&buf, &n);
            else ReadCmdLine(&buf, &n);
            if(connectionOption == 0) {
                CloseAllPipe();
                //ListAllPipe();
                if(remoteOption) break;
                else return;
            }

            /*** cut input ***/
            char buf2[BUF_MAX_SIZE];
            bzero(buf2, BUF_MAX_SIZE);
            strncpy(buf2, buf, n);
            strtok(buf2, "\n");
            strtok(buf2, "\r");
            /*write(STDERR_FILENO, "parse cmd line: ", 16);
            write(STDERR_FILENO, buf2, strlen(buf2));
            write(STDERR_FILENO, "\n", 1);*/

            ParsePipe(buf2, execCmdQueue, &cmdNum);

            int i = 0;
            for(; i < cmdNum; ++i) {

                /*** clean & initialize variables ***/
                bzero(execPath, PATH_MAX_LEN);
                bzero(execCmd, CMD_MAX_LEN);
                bzero(execArgv, sizeof(execArgv));
                argvNum = 0;
                cmdLen = 0;

                if(ParseCmdAndArgv(execCmdQueue[i].execCmdLine, execCmd, execArgv, &cmdLen, &argvNum, &execCmdQueue[i].pipeType) == -1) {
                    break;
                }

                /*** select a function ***/
                if(!strcmp(execCmd, "setenv")) {
                    SetEnv(argvNum, execArgv);
                }
                else if(!strcmp(execCmd, "printenv")) {
                    PrintEnv();
                }
                else if(!strcmp(execCmd, "\n")) {
                    break;
                }
                else if(!strcmp(execCmd, "exit")) {
                    CloseAllPipe();
                    //ListAllPipe();
                    if(remoteOption) {
                        connectionOption = 0;
                        break;
                    }
                    else {
                        return 0;
                    }
                }
                else {
                    /*** prepare to execute cmd ***/

                    int refreshOption = 0;
                    int stdinOption = 0;
                    int stdoutOption = 0;
                    char* ptrNextCmdLine;
                    if(i+1 < cmdNum) {
                        ptrNextCmdLine = execCmdQueue[i+1].execCmdLine;
                    }

                    /*** | or ! or > pipe ***/
                    if(execCmdQueue[i].pipeType > 0) {
                        CreatePipe(&stdoutOption, &execCmdQueue[i].pipeType, ptrNextCmdLine);
                        
                        SetPipePriority(ptrNextCmdLine, &cmdNum, &refreshOption, &execCmdQueue[i].pipeType);
                        
                        //ListAllPipe();
                    }

                    /*** fork a process to execute the cmd (in child process) ***/
                    if( (childPid = fork()) == -1) {
                        write(STDERR_FILENO, "fork() error.\n", 14);
                    }
                    else if(childPid == 0) {
                        /*** child process ***/

                        if(remoteOption) close(sockfd);

                        SetStdin(&stdinOption);

                        SetStdoutAndStderr(&stdoutOption, &execCmdQueue[i].pipeType);

                        Execute(execCmd, execPath, execArgv, &cmdLen);

                        exit(0);
                    }
                    else {
                        /*** parent process ***/

                        wait();

                        /*** stdout check & close ***/
                        if(stdoutOption == 1) {
                            if(execCmdQueue[i].pipeType == 1 || execCmdQueue[i].pipeType == 2) {
                                //printf("close fd[1]: %d\n", pipesTail->pipefd[1]);fflush(stdout);
                                close(pipesTail->pipefd[1]);
                            }
                            else if(execCmdQueue[i].pipeType == 3 && filepipe != NULL) {
                                //printf("close file, fd: %d\n", pipesTail->pipefd[0]);fflush(stdout);
                                fclose(filepipe);
                                filepipe = NULL;
                                struct PipeStruct* tmpPtr = pipes;
                                struct PipeStruct* tmpPrePtr;
                                pipesTail->priority = 1;
                            }
                        }

                        /*** when stdin complete, close both the r/w pipe fd (that priority==1) ***/
                        struct PipeStruct* ptrPipeStruct = pipes;
                        struct PipeStruct* prePtr = NULL;
                        while(ptrPipeStruct != NULL) {
                            if(ptrPipeStruct->priority == 1) {
                                //printf("close fd[0]: %d\n", ptrPipeStruct->pipefd[0]);fflush(stdout);
                                close(ptrPipeStruct->pipefd[0]);
                                if(ptrPipeStruct == pipes) {
                                    /*** delete the first node ***/
   
                                    if(ptrPipeStruct == pipesTail) {
                                        /*** the first node is also the last node ***/
   
                                        free(ptrPipeStruct);
                                        pipes = NULL;
                                        pipesTail = NULL;
                                        break;
                                    }
                                    else {
                                        pipes = ptrPipeStruct->next;
                                        free(ptrPipeStruct);
                                        ptrPipeStruct = pipes;
                                    }
                                }
                                else if(ptrPipeStruct == pipesTail) {
                                    /*** delete the last node ***/

                                    prePtr->next = NULL;
                                    pipesTail = prePtr;
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

                        /*** refresh the next pipe priority ***/
                        if(refreshOption) {
                            pipesTail->priority = pipesTail->priority - 1;
                        }

                        /*** handle > pipe, end this execCmdQueue ***/
                        if(execCmdQueue[i].pipeType == 3) {
                            break;
                        }
                    }
                }
            }
            RefreshPipe();
        }
        if(remoteOption){
            dup2(stdoutfd, STDOUT_FILENO);
            dup2(stderrfd, STDERR_FILENO);
            printf("client left.\n");fflush(stdout);
            close(newsockfd);
        }
        welcomeMsgOption = 1;
        bzero(PATH, sizeof(PATH));
        strcpy(PATH[0], "bin");
        strcpy(PATH[1], ".");
        pathNum = 2;
    }
    if(remoteOption) close(sockfd);
    free(buf);
    return 0;
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
    return 0;
}

void CreateConnection() {

    /*** create socket ***/
    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        write(STDERR_FILENO, "socket error!\n", 14);
        exit(1);
    }
    write(STDERR_FILENO, "socket create\n", 14);

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    /*** bind ***/
    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        write(STDERR_FILENO, "bind error!\n", 12);
        exit(1);
    }
    printf("bind@server: localhost:%d\n", SERV_TCP_PORT);fflush(stdout);

    /*** listen ***/
    listen(sockfd, 1);
    printf("listen. fd: %d\n", sockfd);
    return;
}

/*** read a command line to buffer from localhost ***/
void ReadCmdLine(char** buf, int* n) {
    if( (*n = read(0, *buf, BUF_MAX_SIZE-1)) < 1) {
        write(STDERR_FILENO, "read error!\n", 12);
    }
    else {
        //printf("read %d bytes\n", *n);fflush(stdout);
    }
    return;
}

/*** read a command line to buffer from remote client ***/
int ReadCmdLineFromRemote(char** buf, int* n) {
    char* ptrBuf = *buf;
    int i = 0, result;
    char c;
    /*
    for(; i < BUF_MAX_SIZE-1; ++i) {
        if((result = read(newsockfd, &c, 1)) == 1) {
            if(c == '\r') continue;
            *ptrBuf = c;
            ptrBuf = ptrBuf + 1;
            if(c == '\n') {
                break;
            }
        }
        else if (result == 0) {
            *ptrBuf = '\0';
            return 0;
        }
        else {
            write(STDERR_FILENO, "read error!\n", 12);
            return -1;
        }
    }
    *ptrBuf = '\0';
    *n = ++i;
    */
    while( (result = read(newsockfd, &c, 1)) == 1 ) {
        if(c == '\r') continue;
        if(i == BUF_MAX_SIZE - 1) {
            write(stdoutfd, "\nread overflow!\n", 16);
            char* tmpBuf = *buf;
            *buf = malloc(BUF_MAX_SIZE * 2);
            memcpy(*buf, tmpBuf, BUF_MAX_SIZE);
            free(tmpBuf);
            BUF_MAX_SIZE = BUF_MAX_SIZE * 2;
            ptrBuf = *buf + i;
        }
        *ptrBuf = c;
        ptrBuf = ptrBuf + 1;
        ++i;
        if(c == '\n') break;
    }
    *ptrBuf = '\0';
    *n = ++i;
    if(result == 0) {
        return 0;
    }

    //write(STDERR_FILENO, buf, i);
    //printf("read %d bytes\n", *n);fflush(stdout);
    return i;
}

/*** parse pipe |, !, > ***/
void ParsePipe(char* buf, struct ExecCmd* execCmdQueue, int* cmdNum) {
    char* ptrTmp = buf;
    char* ptrPipe = buf;
    int cmdLineLen = 0;
    for(; (ptrTmp = strpbrk(ptrTmp, "|!>")); *cmdNum = *cmdNum + 1) {
        if( *ptrTmp == '|') {
            ptrPipe = strtok(ptrPipe, "|");
            cmdLineLen = strlen(ptrPipe);
            strncpy(execCmdQueue[*cmdNum].execCmdLine, ptrPipe, cmdLineLen);
            execCmdQueue[*cmdNum].pipeType = 1;
            //write(STDERR_FILENO, "pipe: |\n", 8);
        }
        else if( *ptrTmp == '!'){
            ptrPipe = strtok(ptrPipe, "!");
            cmdLineLen = strlen(ptrPipe);
            strncpy(execCmdQueue[*cmdNum].execCmdLine, ptrPipe, cmdLineLen);
            execCmdQueue[*cmdNum].pipeType = 2;
            //write(STDERR_FILENO, "pipe: !\n", 8);
        }
        else if( *ptrTmp == '>') {
            ptrPipe = strtok(ptrPipe, ">");
            cmdLineLen = strlen(ptrPipe);
            strncpy(execCmdQueue[*cmdNum].execCmdLine, ptrPipe, cmdLineLen);
            execCmdQueue[*cmdNum].pipeType = 3;
            //write(STDERR_FILENO, "pipe: >\n", 8);
        }
        ++ptrTmp;
        ptrPipe = ptrTmp;
        /*write(STDERR_FILENO, "add '", 5);
        write(STDERR_FILENO, execCmdQueue[*cmdNum].execCmdLine, cmdLineLen);
        write(STDERR_FILENO, "' to execCmdQueue\n", 18);*/
    }
    cmdLineLen = strlen(ptrPipe);
    strncpy(execCmdQueue[*cmdNum].execCmdLine, ptrPipe, strlen(ptrPipe));
    /*write(STDERR_FILENO, "add '", 5);
    write(STDERR_FILENO, execCmdQueue[*cmdNum].execCmdLine, cmdLineLen);
    write(STDERR_FILENO, "' to execCmdQueue\n", 18);*/
    *cmdNum = *cmdNum + 1;
    return;
}

/*** parse cmd and parameters (argv) ***/
int ParseCmdAndArgv(char* execCmdLine, char* execCmd, char** execArgv, int* cmdLen, int* argvNum, int* pipeType) {

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
    for(*argvNum = 1; (ptrCmd = strtok(NULL, " ")); *argvNum = *argvNum + 1) {
        execArgv[*argvNum] = ptrCmd;
        //printf("'%s' ", execArgv[*argvNum]);fflush(stdout);
    }

    /*** check whether the last argv is "<<EOF" that is pipeType:6 ***/
    if(*argvNum > 1 && (strcmp(execArgv[*argvNum - 1], "<<EOF") == 0)) {
        *pipeType = 6;
        execArgv[*argvNum - 1] = NULL;
        *argvNum = *argvNum - 1;
    }
    else if(*argvNum > 1 && (strcmp(execArgv[*argvNum - 1], "EOF") == 0) && (strcmp(execArgv[*argvNum - 2], "<<") == 0)) {
        *pipeType = 6;
        execArgv[*argvNum - 1] = NULL;
        execArgv[*argvNum - 2] = NULL;
        *argvNum = *argvNum - 2;
    }

    execArgv[*argvNum] = NULL;
    //printf("\n");fflush(stdout);
    return 0;
}

/*** create a pipe and add it to pipe queue (pipes) ***/
void CreatePipe(int* stdoutOption, int* pipeType, char* nextExecCmdLine) {
    if(*pipeType == 1 || *pipeType == 2) {
        /*** pipe | or !, creating a pipe  ***/

        *stdoutOption = 1;
        struct PipeStruct* ptrPipeStruct = malloc(sizeof(struct PipeStruct));
        if(pipe(ptrPipeStruct->pipefd) < 0) {
            write(STDERR_FILENO, "pipe error!\n", 12);
        }
        if(pipes == NULL) {
            /*** first node ***/

            pipes = ptrPipeStruct;
            pipesTail = ptrPipeStruct;
            ptrPipeStruct->next = NULL;
        }
        else {
            pipesTail->next = ptrPipeStruct;
            pipesTail = ptrPipeStruct;
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
        if(pipes == NULL) {
            /*** first node ***/

            pipes = ptrPipeStruct;
            pipesTail = ptrPipeStruct;
            ptrPipeStruct->next = NULL;
        }
        else {
            pipesTail->next = ptrPipeStruct;
            pipesTail = ptrPipeStruct;
            ptrPipeStruct->next = NULL;
        }
        //printf("open file: %s, fd: %d\n", filename, filefd);fflush(stdout);
    }
    else if(*pipeType == 6) {
        char buffer[BUF_MAX_SIZE];
        int n = 0;

        struct PipeStruct* ptrPipeStruct = malloc(sizeof(struct PipeStruct));
        if(pipe(ptrPipeStruct->pipefd) < 0) {
            write(STDERR_FILENO, "pipe error!\n", 12);
        }
        if(pipes == NULL) {
            /*** first node ***/

            pipes = ptrPipeStruct;
            pipesTail = ptrPipeStruct;
            ptrPipeStruct->next = NULL;
        }
        else {
            pipesTail->next = ptrPipeStruct;
            pipesTail = ptrPipeStruct;
            ptrPipeStruct->next = NULL;
        }
        ptrPipeStruct->priority = 1;
        //printf("open fd[0]: %d, fd[1]: %d\n", ptrPipeStruct->pipefd[0], ptrPipeStruct->pipefd[1]);fflush(stdout);

        while(1) {
            if(remoteOption) ReadCmdLineFromRemote(&buffer, &n);
            else ReadCmdLine(&buffer, &n);

            //write(stdoutfd, buffer, n);

            if( (strcmp(buffer, "EOF\n") == 0) || (strcmp(buffer, "EOF\r\n") == 0)) break;

            write(ptrPipeStruct->pipefd[1], buffer, strlen(buffer));
            bzero(buffer, BUF_MAX_SIZE);
        }
        close(ptrPipeStruct->pipefd[1]);
        ptrPipeStruct->pipefd[1] = -1;
        //write(stdoutfd, "read EOF\n", 9);
    }
    else {
        write(STDERR_FILENO, "invalid pipe!\n", 14);
    }
    return;
}

/*** after add a pipe to pipe queue (pipes), set the pipe priority for the pipe ***/
void SetPipePriority(char* nextExecCmdLine, int* cmdNum, int* refreshOption, int* pipeType) {
    struct PipeStruct* ptrPipeStruct = pipesTail;
    if(*pipeType == 3) {
        /*** pipe >, priority = 0 (not for other cmd) ***/
        ptrPipeStruct->priority = 0;
    }
    else if(*pipeType == 6) {
        /*** pipe <<EOF, priority = 1 ***/
        ptrPipeStruct->priority = 1;
    }
    else {
        /*** pipe | or ! ***/

        /*** check whether the next cmd is \s*[0-9]* or not ***/
        int regexFlags = REG_EXTENDED | REG_ICASE;
        regex_t preg;
        size_t nMatch = 1;
        regmatch_t pMatch[nMatch];
        char* pattern = "^\\s*[0-9]*$";
        int result;
        if(regcomp(&preg, pattern, regexFlags) != 0) {
            /*** initialize Regex ***/

            write(STDERR_FILENO, "Regex compile error!\n", 21);
        }
        else if( (result = regexec(&preg, nextExecCmdLine, nMatch, pMatch, 0)) == 0) {
            /*** match, pipe priority is the number ***/

            regfree(&preg);
            ptrPipeStruct->priority = atoi(nextExecCmdLine) + 1;
            *cmdNum = *cmdNum - 1;
        }
        else {
            /*** not match, the next is a normal command ***/

            regfree(&preg);
            ptrPipeStruct->priority = 2;
            *refreshOption = 1;
        }
    }
    return;
}

/*** stdin check & dup ***/
void SetStdin(int* stdinOption) {
    int tmpPipefd[2];
    struct PipeStruct* ptrPipeStruct = pipes;
    while(ptrPipeStruct != NULL) {
        if(ptrPipeStruct->priority == 1) {
            if(*stdinOption == 0) {
                *stdinOption = 1;
                if(pipe(tmpPipefd) < 0) {
                    write(STDERR_FILENO, "tmp pipe error!\n", 12);
                }
                //printf("(tmp pipe) open fd[0]: %d, fd[1]: %d\n", tmpPipefd[0], tmpPipefd[1]);fflush(stdout);
                //printf("indexStdin: ");fflush(stdout);
            }
            //printf("stdin: fd[0]: %d ", ptrPipeStruct->pipefd[0]);fflush(stdout);
            int n;
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
        //printf("\n");fflush(stdout);
        close(tmpPipefd[1]);
        dup2(tmpPipefd[0], STDIN_FILENO);
    }
    return;
}

/*** stdout/stderr check & dup ***/
void SetStdoutAndStderr(int* stdoutOption, int* pipeType) {

    /*** stdout check & dup ***/
    if(*stdoutOption) {
        //printf("indexStdout: %d\n", indexStdout);fflush(stdout);
        
        /*** pipe | or !, close unuse pipefd ***/
        if(pipesTail->priority != 0) {
            close(pipesTail->pipefd[0]);
        }

        dup2(pipesTail->pipefd[1], STDOUT_FILENO);

        /*** stderr check & dup ***/
        if(*pipeType == 2) {
            dup2(pipesTail->pipefd[1], STDERR_FILENO);
        }
    }
    return;
}

/*** execute the input cmd ***/
void Execute(char* execCmd, char* execPath, char** execArgv, int* cmdLen) {

    /*** parse execute path ***/
    int find = 0;
    int i;
    char testPath[PATH_MAX_LEN] = "";
    FILE* fp;
    for(i = 0; i < pathNum; ++i) {
        strcat(testPath, PATH[i]);
        strcat(testPath, "/");
        strcat(testPath, execCmd);
        if( (fp = fopen(testPath, "r")) ) {
            fclose(fp);
            strcpy(execPath, testPath);
            execArgv[0] = execPath;
            find = 1;
            /*write(STDERR_FILENO, "cmd path: ", 10);
            write(STDERR_FILENO, execPath, strlen(execPath));
            write(STDERR_FILENO, "\n", 1);*/
            break;
        }
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

/*** cmd printenv ***/
void PrintEnv() {
    int i = 0, pathLen, pathNum = PATH_MAX_NUM;
    write(STDOUT_FILENO, "PATH=", 5);
    if( (pathLen = strlen(PATH[i])) > 0) {
        write(STDOUT_FILENO, PATH[i], pathLen);
        for(i = 1, pathLen = strlen(PATH[i]); i < pathNum && pathLen > 0; pathLen = strlen(PATH[++i])) {
            write(STDOUT_FILENO, ":", 1);
            write(STDOUT_FILENO, PATH[i], pathLen);
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

            bzero(PATH, sizeof(PATH));
            pathNum = 0;
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
                bzero(PATH, sizeof(PATH));
                char* ptr = strtok(argv[2], ":");
                strncpy(PATH[0], ptr, PATH_MAX_LEN);
                int i = 1;
                for(; (ptr = strtok(NULL, ":")) && (i < PATH_MAX_NUM); ++i) {
                    strncpy(PATH[i], ptr, PATH_MAX_LEN);
                }
                pathNum = i;
            }
        }
    }
    else {
        regfree(&preg);
        write(STDERR_FILENO, "usage: setenv PATH [path1:path2:...]\n", 37);
    }
    return;
}

/*** refresh all pipes states ***/
void RefreshPipe() {
    struct PipeStruct* ptrPipeStruct = pipes;
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
    struct PipeStruct* ptrPipeStruct = pipes;
    struct PipeStruct* tmpPtr;
    //write(STDERR_FILENO, "close all pipes\n", 16);
    while(ptrPipeStruct != NULL) {
        //printf("close fd[0]: %d\n", ptrPipeStruct->pipefd[0]);fflush(stdout);
        close(ptrPipeStruct->pipefd[0]);
        //printf("close fd[1]: %d\n", ptrPipeStruct->pipefd[1]);fflush(stdout);
        close(ptrPipeStruct->pipefd[1]);
        tmpPtr = ptrPipeStruct;
        ptrPipeStruct = ptrPipeStruct->next;
        free(tmpPtr);
    }
    pipes = NULL;
    pipesTail = NULL;
    //write(STDERR_FILENO, "close all pipes in pipe queue (pipes)\n", 38);
    return;
}

void ListAllPipe() {
    struct PipeStruct* ptrPipeStruct = pipes;
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
    return;
}

