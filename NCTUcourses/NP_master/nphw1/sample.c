/******
 *
 * 'NP hw1 practice sample'
 * by headhsu
 *
 ******/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

/*** Define value ***/
#define PATH_MAX_NUM 100
#define PATH_MAX_LEN 1000
#define CMD_MAX_LEN 100
#define BUF_MAX_SIZE 2000
#define CMD_MAX_NUM 100
#define PIPE_MAX_NUM 1000

/*** data structure definition ***/
struct PipeStruct {
    int pipefd[2];
    int priority;
};
struct ExecCmd {
    char execCmdLine[CMD_MAX_LEN];

    /*** 0:none(default), 1:|, 2:!, 3:> ***/
    int pipeType;
};

/*** environment parameters ***/
char PATH[PATH_MAX_NUM][PATH_MAX_LEN] = {"bin", "."};
int welcomeMsgOption = 1;
struct PipeStruct pipes[PIPE_MAX_NUM];

/*** function delcare ***/
int WelcomeMsg();
void Execute(char* execCmd, char* execPath, char** execArgv, int* cmdLen);
void PrintEnv();
void SetEnv(int argc, char** argv);
int FindEmptyPipe();
int FindStdinPipe(int* indexStdin);
void RefreshPipe();
void CloseAllPipe();

int main(){
    int n, childPid;
    char buf[BUF_MAX_SIZE];
    char execPath[PATH_MAX_LEN];
    char execCmd[CMD_MAX_LEN];
    int cmdLen = 0;
    char* execArgv[BUF_MAX_SIZE];
    int argvNum = 0;
    struct ExecCmd execCmdQueue[CMD_MAX_NUM];
    int cmdNum = 0;

    /*** shell start ***/
    while(1) {
        if(welcomeMsgOption) {
            welcomeMsgOption = WelcomeMsg();
        }
        write(STDOUT_FILENO, "% ", 2);

        /*** read cmd line ***/
        bzero(buf, BUF_MAX_SIZE);
        bzero(execPath, PATH_MAX_LEN);
        bzero(execCmd, CMD_MAX_LEN);
        bzero(execArgv, sizeof(execArgv));
        bzero(execCmdQueue, sizeof(execCmdQueue));
        cmdNum = 0;
        argvNum = 0;
        cmdLen = 0;
        if( (n = read(0, buf, sizeof(buf))) < 1) {
            //printf("read error!\n");
        }
        else {
            //printf("read %d bytes\n", n);
        }
        char buf2[BUF_MAX_SIZE];
        strncpy(buf2, buf, n);
        strtok(buf2, "\n");

        /*** parse pipe |, !, > ***/
        char* ptrTmp = buf2;
        char* ptrPipe = buf2;
        for(; (ptrTmp = strpbrk(ptrTmp, "|!>")); ++cmdNum) {
            if( *ptrTmp == '|') {
                ptrPipe = strtok(ptrPipe, "|");
                strcpy(execCmdQueue[cmdNum].execCmdLine, ptrPipe);
                execCmdQueue[cmdNum].pipeType = 1;
                //printf("pipe: |\n");
            }
            else if( *ptrTmp == '!'){
                ptrPipe = strtok(ptrPipe, "!");
                strcpy(execCmdQueue[cmdNum].execCmdLine, ptrPipe);
                execCmdQueue[cmdNum].pipeType = 2;
                //printf("pipe: !\n");
            }
            else if( *ptrTmp == '>') {
                ptrPipe = strtok(ptrPipe, ">");
                strcpy(execCmdQueue[cmdNum].execCmdLine, ptrPipe);
                execCmdQueue[cmdNum].pipeType = 3;
                //printf("pipe: >\n");
            }
            ++ptrTmp;
            ptrPipe = ptrTmp;
        }
        strcpy(execCmdQueue[cmdNum].execCmdLine, ptrPipe);
        ++cmdNum;

        int i = 0;
        for(; i < cmdNum; ++i) {
            bzero(execPath, PATH_MAX_LEN);
            bzero(execCmd, CMD_MAX_LEN);
            bzero(execArgv, sizeof(execArgv));
            argvNum = 0;
            cmdLen = 0;

            /*** parse cmd ***/
            char* ptrCmd;
            if( !(ptrCmd = strtok(execCmdQueue[i].execCmdLine, " "))) {
                break;
            }
            cmdLen = strlen(ptrCmd);
            strcpy(execCmd, ptrCmd);
            //printf("cmd: %s, cmdLen: %d", execCmd, cmdLen);

            /*** parse parameter (argv) ***/
            //printf(", argv: ");
            for(argvNum = 1; (ptrCmd = strtok(NULL, " ")); ++argvNum) {
                execArgv[argvNum] = ptrCmd;
                //printf("'%s' ", execArgv[argvNum]);
            }
            //printf("\n");

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
                return 0;
            }
            else {
                int indexStdout = -1;
                int indexStdin[PIPE_MAX_NUM] = {-1};
                int refreshOption = 0;
                if(execCmdQueue[i].pipeType == 1 || execCmdQueue[i].pipeType == 2) {
                    /*** | or ! pipe, creating a pipe ***/

                    if( (indexStdout = FindEmptyPipe()) == -1) {
                        write(STDERR_FILENO, "no space to create a pipe!\n", 27);
                    }

                    if(pipe(pipes[indexStdout].pipefd) < 0) {
                        write(STDERR_FILENO, "pipe error!\n", 12);
                    }

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
                    else if( (result = regexec(&preg, execCmdQueue[i+1].execCmdLine, nMatch, pMatch, 0)) == 0) {
                        /*** match, pipe priority is the number ***/

                        regfree(&preg);
                        pipes[indexStdout].priority = atoi(execCmdQueue[i+1].execCmdLine) + 1;
                        --cmdNum;
                    }
                    else {
                        /*** not match, the next is a normal command ***/

                        regfree(&preg);
                        pipes[indexStdout].priority = 2;
                        refreshOption = 1;
                    }
                }

                if( (childPid = fork()) == -1) {
                    write(STDERR_FILENO, "fork() error.\n", 14);
                }
                else if(childPid == 0) {
                    /*** child process ***/

                    /*** stdin check & dup ***/
                    int stdinNum = FindStdinPipe(indexStdin);
                    int tmpPipefd[2];
                    if(pipe(tmpPipefd) < 0) {
                        write(STDERR_FILENO, "pipe error!\n", 12);
                    }
                    if( indexStdin[0] != -1 ) {
                        int j = 0;
                        //printf("indexStdin: ");
                        for(; j < stdinNum; ++j) {
                            //printf("%d ", indexStdin[j]);
                            int n;
                            char bufTmp[BUF_MAX_SIZE];
                            while(n = read(pipes[indexStdin[j]].pipefd[0], bufTmp, sizeof(bufTmp))) {
                                write(tmpPipefd[1], bufTmp, n);
                                bzero(bufTmp, sizeof(bufTmp));
                            }
                            close(pipes[indexStdin[j]].pipefd[0]);
                        }
                        //printf("\n");
                        close(tmpPipefd[1]);
                        dup2(tmpPipefd[0], STDIN_FILENO);
                    }

                    /*** stdout check & dup ***/
                    if( indexStdout != -1) {
                        //printf("indexStdout: %d\n", indexStdout);
                        close(pipes[indexStdout].pipefd[0]);
                        dup2(pipes[indexStdout].pipefd[1], STDOUT_FILENO);

                        /*** stderr check & dup ***/
                        if(execCmdQueue[i].pipeType == 2) {
                            dup2(pipes[indexStdout].pipefd[1], STDERR_FILENO);
                        }
                    }

                    /*** stdout check & dup for > pipe ***/
                    char filename[CMD_MAX_LEN];
                    FILE* fp;
                    int filefd;
                    if(execCmdQueue[i].pipeType == 3){
                        char* ptrFilename = strtok(execCmdQueue[i+1].execCmdLine, " ");
                        strcpy(filename, ptrFilename);
                        fp = fopen(filename, "a");
                        filefd = fileno(fp);
                        dup2(filefd, STDOUT_FILENO);
                    }

                    Execute(execCmd, execPath, execArgv, &cmdLen);

                    /*** when stdin complete, close both the r/w pipe fd ***/
                    if( indexStdin[0] != -1 ) {
                        close(tmpPipefd[0]);
                    }

                    /*** stdout check & close ***/
                    if( indexStdout != -1 ) {
                        close(pipes[indexStdout].pipefd[1]);
                    }
                    if(execCmdQueue[i].pipeType == 3){
                        fclose(fp);
                    }

                    exit(0);
                }
                else {
                    /*** parent process ***/

                    wait();
                    
                    /*** when stdin complete, close both the r/w pipe fd ***/
                    if( indexStdin[0] != -1 ) {
                        int j = 0;
                        for(; j < PIPE_MAX_NUM && indexStdin[j] != -1; ++j) {
                            close(pipes[indexStdin[j]].pipefd[1]);
                            pipes[indexStdin[j]].priority = 0;
                        }
                    }

                    /*** stdout check & close ***/
                    if( indexStdout != -1 ) {
                        close(pipes[indexStdout].pipefd[1]);
                    }

                    /*** refresh the next pipe priority ***/
                    if(refreshOption) {
                        --pipes[indexStdout].priority;
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

    return 0;
}

/*** show welcome message ***/
int WelcomeMsg() {
    write(STDOUT_FILENO, "***************************************************************\n", 64);
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
    write(STDOUT_FILENO, "**                                                             \n", 64);
    return 0;
}

/*** execute the input cmd ***/
void Execute(char* execCmd, char* execPath, char** execArgv, int* cmdLen) {

    /*** parse execute path ***/
    int pathNum = PATH_MAX_NUM, i;
    char testPath[PATH_MAX_LEN] = "";
    FILE* fp;
    for(i = 0; i < pathNum; ++i) {
        strcat(testPath, PATH[i]);
        strcat(testPath, "/");
        strcat(testPath, execCmd);
        if( (fp = fopen(testPath, "r+")) ) {
            fclose(fp);
            strcpy(execPath, testPath);
            execArgv[0] = execPath;
            //printf("cmd path: %s\n", execPath);
            break;
        }
    }

    /*** execute cmd ***/
    if(execv(execPath, execArgv)) {
        write(STDERR_FILENO, "Unknown command: [", 18);
        write(STDERR_FILENO, execCmd, *cmdLen);
        write(STDERR_FILENO, "]\n", 2);
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
        }
        else {
            /*** setenv PATH [path1:path2:...] ***/

            bzero(PATH, sizeof(PATH));
            char* ptr = strtok(argv[2], ":");
            strncpy(PATH[0], ptr, PATH_MAX_LEN);
            int i = 1;
            for(; (ptr = strtok(NULL, ":")) && (i < PATH_MAX_NUM); ++i) {
                strncpy(PATH[i], ptr, PATH_MAX_LEN);
            }
        }
    }
    else {
        regfree(&preg);
        write(STDERR_FILENO, "usage: setenv PATH [path1:path2:...]\n", 37);
    }
    return;
}

/*** return an index of empty space in pipes (PipeStruct) ***/
int FindEmptyPipe() {
    int i = 0;
    for(; i < PIPE_MAX_NUM; ++i) {
        if(pipes[i].priority == 0) {
            return i;
        }
    }
    return -1;
}

/*** return an index of stdin pipe in pipes (PipeStruct) ***/
int FindStdinPipe(int* indexStdin) {
    int i = 0, j = 0;
    for(; i < PIPE_MAX_NUM; ++i) {
        if(pipes[i].priority == 1) {
            pipes[i].priority = -1;
            indexStdin[j] = i;
            ++j;
        }
    }
    indexStdin[j] = -1;
    return j;
}

/*** refresh all pipes states ***/
void RefreshPipe() {
    int i = 0;
    for(; i < PIPE_MAX_NUM; ++i) {
        if(pipes[i].priority == -1) {
            pipes[i].priority = 0;
        }
        else if(pipes[i].priority > 0) {
            pipes[i].priority = pipes[i].priority-1;
        }
    }
    return;
}

/*** force close all pipes in pipes (PipeStruct) ***/
void CloseAllPipe() {
    int i = 0;
    for(; i < PIPE_MAX_NUM; ++i) {
        if(pipes[i].pipefd[0] != 0) {
            close(pipes[i].pipefd[0]);
        }
        if(pipes[i].pipefd[1] != 0) {
            close(pipes[i].pipefd[1]);
        }
    }
    return;
}

