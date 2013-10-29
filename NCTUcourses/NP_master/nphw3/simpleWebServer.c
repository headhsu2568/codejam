/******
 *
 * 'NP simple web server'
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
#include <sys/signal.h>
#include <errno.h>

/*** Define value ***/
#define BUF_MAX_SIZE 10240
#define SERV_TCP_PORT 7774
#define LISTEN_MAX_NUM 5
#define HEADER_MAX_LEN 1024
#define HEADER_NUM 9

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
char* WorkDir = "./";
int stdoutfd = STDOUT_FILENO;
int stderrfd = STDERR_FILENO;
int clientCount = 0;
int curClientNo = 0;
struct ClientInfo curClient;

/*** connection variables ***/
struct sockaddr_in serv_addr, cli_addr;
int listenfd, connfd;
int clilen, childPid;
char* contentType = "Content-type: text/html\r\n";
char* status200 = "Status: 200 OK\r\n\r\n";
char request[HEADER_NUM][HEADER_MAX_LEN];
char buffer[BUF_MAX_SIZE];

/*** signal function declare ***/

/*** function delcare ***/
void CreateConnection();
void NewClient();
int ReadRequest();
void ParseMethod(char* method, char* target, char* queryString);
void ExectueMethod();
void Disconnect();
void ListRequest();

int main() {
    int res = 0;

    //chdir(WorkDir);
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
                write(curClient.fd, contentType, strlen(contentType));
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
                    ExectueMethod();

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
    return 0;
}

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
        printf("read request %d bytes\n", n);fflush(stdout);
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
        printf("method: %s\n", method);fflush(stdout);
        
        if((ptrReq = strtok(NULL, " "))) {
            char* ptrTar = ptrReq;

            ptrReq = strtok(ptrReq, "?");
            if((ptrReq = strtok(NULL, "?"))) {
                strcpy(target, ptrTar);
                printf("target: %s\n", target);fflush(stdout);
                strcpy(queryString, ptrReq);
                printf("query string: %s\n", queryString);fflush(stdout);
            }
            else {
                strcpy(target, ptrTar);
                printf("target: %s\n", target);fflush(stdout);
                strcpy(queryString, "");
                printf("query string: %s(empty)\n", queryString);fflush(stdout);
            }
        }
    }
    return;
}

/*** execute the action of method header ***/
void ExectueMethod() {
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
