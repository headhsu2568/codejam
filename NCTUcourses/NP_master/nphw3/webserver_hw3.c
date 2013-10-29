/******
 *
 * 'NP hw3 http server'
 * by headhsu
 *
 * gcc -fno-stack-protector -o webserver_hw3 webserver_hw3.c
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
#include <sys/signal.h>
#include <errno.h>

/*** Define value ***/
#define BUF_MAX_SIZE 40960
#define SERV_TCP_PORT 7774
#define LISTEN_MAX_NUM 5
#define HEADER_MAX_LEN 1024
#define HEADER_NUM 9
#define ARGV_MAX_NUM 3
#define ENVPARAM_NUM 9
#define ENVPARAM_MAX_LEN 1500

/*** Define request header index ***/
#define METHOD 0
#define HOST 1
#define CONNECTION 2
#define USERAGENT 3
#define ACCEPT 4
#define ACCEPTENCODING 5
#define ACCEPTLANGUAGE 6
#define ACCEPTCHARSET 7
#define OTHER 8

/*** Define environment parameters(envParam[]) index ***/
#define QUERY_STRING 0
#define CONTENT_LENGTH 1
#define REQUEST_METHOD 2
#define SCRIPT_NAME 3
#define REMOTE_HOST 4
#define REMOTE_ADDR 5
#define ANTH_TYPE 6
#define REMOTE_USER 7
#define REMOTE_IDENT 8

/*** data structure definition ***/
struct ClientInfo {

    /*** connection variables ***/
    int fd;
    int clilen;
    struct sockaddr_in cli_addr;
    unsigned int port;
    char ip[16];

    /*** user environment ***/
    int pid;
    int isConnect;
};

/*** environment parameters ***/
char* workDir = "webroot";
int stdoutfd = STDOUT_FILENO;
int stderrfd = STDERR_FILENO;
int clientCount = 0;
int curClientNo = 0;
struct ClientInfo curClient;
char envParam[ENVPARAM_NUM][ENVPARAM_MAX_LEN];

/*** connection variables ***/
struct sockaddr_in serv_addr, cli_addr;
int listenfd, connfd;
int clilen, childPid;
char* status200 = "HTTP/1.1 200 OK";
char* responseHeader = "Content-type: text/html\r\nStatus: 200 OK\r\n\r\n";
char request[HEADER_NUM][HEADER_MAX_LEN];
char buffer[BUF_MAX_SIZE];

/*** signal function declare ***/
void SigChild(int signo);

/*** function delcare ***/
void CreateConnection();
void NewClient();
int ReadRequest();
void ParseMethod(char* method, char* target, char* queryString);
void ExecuteMethod(char* method, char* target, char* queryString);
void SetEnvParam(char** envp, char* method, char* target, char* queryString);
void Disconnect();
void ListRequest();

int main() {
    int res = 0;

    chdir(workDir);
    signal(SIGCHLD, SigChild);
    stdoutfd = dup(STDOUT_FILENO);
    stderrfd = dup(STDERR_FILENO);
    CreateConnection();

    /*** when a connection comes ***/
    for(;;) {

        clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
        if(clientCount < LISTEN_MAX_NUM) {

            if(connfd < 0) {
                printf("error: accept error (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
                close(connfd);
                bzero(&curClient, sizeof(curClient));
                bzero(&cli_addr, sizeof(cli_addr));
                clilen = 0;
                connfd = 0;
            }
            else {
                printf("accept\n");fflush(stdout);

                NewClient();

                /*** read request ***/
                if( (res = ReadRequest()) < 0) continue;

                /*** send status response ***/
                write(curClient.fd, status200, strlen(status200));

                /*** fork a child process for client ***/
                if( (childPid = fork()) == -1) {
                    printf("error: fork() error (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
                    Disconnect();
                }
                else if(childPid == 0) {
                    /*** child process: client work environment ***/

                    close(listenfd);
                    listenfd = -1;

                    /*** parse headers ***/
                    char method[5];
                    char target[HEADER_MAX_LEN];
                    char queryString[HEADER_MAX_LEN];
                    ParseMethod(method, target, queryString);

                    /*** execute method ***/
                    ExecuteMethod(method, target, queryString);

                    exit(0);
                }
                else {
                    /*** parent process ***/

                    close(connfd);
                    bzero(&curClient, sizeof(curClient));
                    bzero(&cli_addr, sizeof(cli_addr));
                    clilen = 0;
                    connfd = 0;
                    bzero(buffer, BUF_MAX_SIZE);
                    bzero(request, sizeof(request));
                }
            }
        }
        else {
            printf("error: too many clients now (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
            close(connfd);
            bzero(&cli_addr, sizeof(cli_addr));
            clilen = 0;
            connfd = 0;
        }
    }

    wait();
    return 0;
}

/*** SIGCHLD handler ***/
void SigChild(int signo) {
    if(childPid > 0) {
        --clientCount;
    }
    return;
}

/*** start the web server listener ***/
void CreateConnection() {

    /*** create socket ***/
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        printf("error: socket error (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
        exit(1);
    }
    printf("socket create\n");fflush(stdout);

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    /*** bind ***/
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("error: bind errorr (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
        exit(1);
    }
    printf("bind@server: localhost:%d\n", SERV_TCP_PORT);fflush(stdout);

    /*** listen ***/
    listen(listenfd, LISTEN_MAX_NUM);
    printf("listen. fd: %d\n", listenfd);fflush(stdout);

    return;
}

/*** initialize a new client after accepting a connection ***/
void NewClient() {

    /*** initialize client (struct ClientInfo) ***/
    curClient.fd = connfd;
    curClient.clilen = clilen;
    curClient.cli_addr = cli_addr;
    curClient.port = htons(curClient.cli_addr.sin_port);
    strncpy(curClient.ip, inet_ntoa(curClient.cli_addr.sin_addr), 16);
    curClient.isConnect = 1;
    ++clientCount;
    ++curClientNo;

    printf("new client #%d (fd: %d, online: %d)\n", curClientNo, curClient.fd, clientCount);fflush(stdout);

    return;
}

/*** read request from client and store to array ***/
int ReadRequest() {
    int n = 0, i = 0;
    char* ptrBuf = buffer;
    if( (n = read(curClient.fd, buffer, BUF_MAX_SIZE-1)) > 0) {
        buffer[n] = '\0';
        //printf("read request %d bytes\n", n);fflush(stdout);
        //write(STDOUT_FILENO, "request: \n", 10);
        //write(STDOUT_FILENO, buffer, n);

        if((ptrBuf = strtok(ptrBuf, "\n"))) {
            strncpy(request[i], ptrBuf, HEADER_MAX_LEN-1);
            ++i;
            while((ptrBuf = strtok(NULL, "\n")) && i < HEADER_NUM-1) {
                strncpy(request[i], ptrBuf, HEADER_MAX_LEN-1);
                ++i;
            }
            if(i == HEADER_NUM-1) {
                strncpy(request[i], ptrBuf, HEADER_MAX_LEN-1);
                ++i;
            }
        }
        else strncpy(request[i], ptrBuf, HEADER_MAX_LEN-1);

        //ListRequest();
        bzero(buffer, BUF_MAX_SIZE);
    }
    else if(n == 0) {
        Disconnect();
        return -1;
    }
    else if (n < 0) {
        printf("error: read error (errno: %d - %s)\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

/*** parse the method header after read request ***/
void ParseMethod(char* method, char* target, char* queryString) {
    char tmpReq[HEADER_MAX_LEN];
    strncpy(tmpReq, request[METHOD], HEADER_MAX_LEN);
    char* ptrReq = tmpReq;
    if((ptrReq = strtok(ptrReq, " "))) {
        strncpy(method, ptrReq, 4);
        //printf("method: %s\n", method);fflush(stdout);
        
        if((ptrReq = strtok(NULL, " "))) {
            char* ptrTar = ptrReq;

            ptrReq = strtok(ptrReq, "?");
            if((ptrReq = strtok(NULL, "?"))) {
                strcpy(target, ptrTar);
                //printf("target: %s\n", target);fflush(stdout);
                strcpy(queryString, ptrReq);
                //printf("query string: %s\n", queryString);fflush(stdout);
            }
            else {
                strcpy(target, ptrTar);
                //printf("target: %s\n", target);fflush(stdout);
                strcpy(queryString, "");
                //printf("query string: %s(empty)\n", queryString);fflush(stdout);
            }
        }
    }
    return;
}

/*** execute the action of method header ***/
void ExecuteMethod(char* method, char* target, char* queryString) {
    char* targetExt = strrchr(target, '.');
    char targetPath[HEADER_MAX_LEN + strlen(workDir)];
    char* argv[ARGV_MAX_NUM];
    char* envp[ENVPARAM_NUM];

    /*** set target path ***/
    strcpy(targetPath, target+1);
    printf("set target path: %s\n", targetPath);

    if(strcmp(method, "GET") == 0) {
        /*** method: GET ***/

        /*** set environment parameters ***/
        SetEnvParam(envp, method, target, queryString);

        /*** parse target file extension and execute ***/
        if(targetExt != NULL) {
            if((strcmp(targetExt, ".cgi") == 0) || (strcmp(targetExt, ".fcgi") == 0)) {
                argv[0] = targetPath;
                argv[1] = NULL;
                printf("execute %s program\n", targetExt);fflush(stdout);

                /*** redirect the stadout pipe to client fd ***/
                dup2(curClient.fd, STDOUT_FILENO);

                /*** execute ***/
                if(execve(targetPath, argv, envp) == -1) {
                    printf("error: execv failed (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
                }
            }
            else if((strcmp(targetExt, ".html") == 0) || strcmp(targetExt, ".htm") == 0) {

                /*** send response headers ***/
                write(curClient.fd, responseHeader, strlen(responseHeader));

                /*** send response content ***/
                FILE* fp;
                if(fp = fopen(targetPath, "r")) {
                    printf("output %s web page\n", targetPath);fflush(stdout);
                    char buf[BUF_MAX_SIZE];
                    int n = 0;
                    while(!feof(fp)) {
                        n = fread(buf, sizeof(char), BUF_MAX_SIZE, fp);
                        if(n <= 0) break;
                        write(curClient.fd, buf, n);
                        bzero(buf, BUF_MAX_SIZE);
                    }
                    fclose(fp);
                }
                else {
                    printf("error: open file failed (errno: %d - %s)\n", errno, strerror(errno));
                }
            }
            else{
            }
        }
        else {
        }
    }
    else if(strcmp(method, "POST") == 0){
        /*** method: POST ***/
    }
    else {
        printf("error: unknown method\n");fflush(stdout);
    }
    return;
}

/*** set environment parameters (envParam[]) for execve later ***/
void SetEnvParam(char** envp, char* method, char* target, char* queryString) {

    /*** QUERY_STRING ***/
    strcpy(envParam[QUERY_STRING], "QUERY_STRING=");
    strncat(envParam[QUERY_STRING], queryString, ENVPARAM_MAX_LEN-strlen(envParam[QUERY_STRING])-1);
    envp[QUERY_STRING] = envParam[QUERY_STRING];

    /*** CONTENT_LENGTH ***/
    strcpy(envParam[CONTENT_LENGTH], "CONTENT_LENGTH=");
    strncat(envParam[CONTENT_LENGTH], "32767", ENVPARAM_MAX_LEN-strlen(envParam[CONTENT_LENGTH])-1);
    envp[CONTENT_LENGTH] = envParam[CONTENT_LENGTH];

    /*** REQUEST_METHOD ***/
    strcpy(envParam[REQUEST_METHOD], "REQUEST_METHOD=");
    strncat(envParam[REQUEST_METHOD], method, ENVPARAM_MAX_LEN-strlen(envParam[REQUEST_METHOD])-1);
    envp[REQUEST_METHOD] = envParam[REQUEST_METHOD];

    /*** SCRIPT_NAME ***/
    strcpy(envParam[SCRIPT_NAME], "SCRIPT_NAME=");
    strncat(envParam[SCRIPT_NAME], target, ENVPARAM_MAX_LEN-strlen(envParam[SCRIPT_NAME])-1);
    envp[SCRIPT_NAME] = envParam[SCRIPT_NAME];
    
    /*** REMOTE_HOST ***/
    strcpy(envParam[REMOTE_HOST], "REMOTE_HOST=");
    strncat(envParam[REMOTE_HOST], "LOCALHOST", ENVPARAM_MAX_LEN-strlen(envParam[REMOTE_HOST])-1);
    envp[REMOTE_HOST] = envParam[REMOTE_HOST];

    /*** REMOTE_ADDR ***/
    strcpy(envParam[REMOTE_ADDR], "REMOTE_ADDR=");
    strncat(envParam[REMOTE_ADDR], curClient.ip, ENVPARAM_MAX_LEN-strlen(envParam[REMOTE_ADDR])-1);
    envp[REMOTE_ADDR] = envParam[REMOTE_ADDR];

    /*** ANTH_TYPE ***/
    strcpy(envParam[ANTH_TYPE], "ANTH_TYPE=");
    strncat(envParam[ANTH_TYPE], "yes", ENVPARAM_MAX_LEN-strlen(envParam[ANTH_TYPE])-1);
    envp[ANTH_TYPE] = envParam[ANTH_TYPE];

    /*** REMOTE_USER ***/
    strcpy(envParam[REMOTE_USER], "REMOTE_USER=");
    strncat(envParam[REMOTE_USER], "Nick", ENVPARAM_MAX_LEN-strlen(envParam[REMOTE_USER])-1);
    envp[REMOTE_USER] = envParam[REMOTE_USER];

    /*** REMOTE_IDENT ***/
    strcpy(envParam[REMOTE_IDENT], "REMOTE_IDENT=");
    strncat(envParam[REMOTE_IDENT], curClient.ip, ENVPARAM_MAX_LEN-strlen(envParam[REMOTE_IDENT])-1);
    envp[REMOTE_IDENT] = envParam[REMOTE_IDENT];

    return;
}

/*** close the connection of the current client ***/
void Disconnect() {
    close(curClient.fd);
    bzero(&curClient, sizeof(curClient));
    bzero(&cli_addr, sizeof(cli_addr));
    clilen = 0;
    connfd = 0;
    bzero(buffer, BUF_MAX_SIZE);
    bzero(request, sizeof(request));
    --clientCount;
    return;
}

/*** show request ***/
void ListRequest() {
    int i = 0;
    printf("\n");fflush(stdout);
    for(; i < HEADER_NUM && strlen(request[i]); ++i) {
        printf("%s\n", request[i]);fflush(stdout);
    }
    return;
}
