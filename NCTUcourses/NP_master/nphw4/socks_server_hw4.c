/******
 *
 * 'NP hw4 SOCKS server'
 * by headhsu
 *
 * gcc -fno-stack-protector -o socks_server_hw4 socks_server_hw4.c
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
#include <fcntl.h>
#include <errno.h>
#include "sem_and_shm.h"

/*** Define value ***/
#define BUF_MAX_SIZE 10240
#define USERID_MAX_LEN 32
#define DOMAIN_NAME_MAX_LEN 32
#define SERV_TCP_PORT 7776
#define BIND_TCP_PORT 9900
#define BIND_MAX_NUM 100
#define LISTEN_MAX_NUM 100
#define RULE_MAX_LEN 128

/*** Define SOCKS command number ***/
#define CONNECT 1
#define BIND 2
#define GRANT 90
#define REJECT 91

/*** Define connection status ***/
#define CONNECTING 1
#define ACTIVE 2

/*** data structure definition ***/
struct SOCKS4_Request {
    unsigned char VN;
    unsigned char CD;
    unsigned char DST_PORT[2];
    unsigned char DST_IP[4];
    unsigned char USER_ID[USERID_MAX_LEN];
    unsigned char UNUSE;
    unsigned char DOMAIN_NAME[DOMAIN_NAME_MAX_LEN];
    unsigned char UNUSE2;
};
struct RequestInfo {
    unsigned char VN;
    unsigned char CD;
    unsigned int DST_PORT;
    unsigned char DST_IP[16];
    unsigned int SRC_PORT;
    unsigned char SRC_IP[16];
    unsigned char* USER_ID;
    unsigned char* DOMAIN_NAME;
};
struct SOCKS4_Reply {
    unsigned char VN;
    unsigned char CD;
    unsigned char DST_PORT[2];
    unsigned char DST_IP[4];
};

/*** environment parameters ***/
char* workDir = "./";
char* socksConfig = "./socks.conf";
unsigned char SOCKS_SERVER_IP[4] = {140, 113, 179, 236};
int stdoutfd = STDOUT_FILENO;
int stderrfd = STDERR_FILENO;
int clientCount = 0;

/*** connection variables ***/
struct sockaddr_in serv_addr, src_addr, dst_addr;
int listenfd, srcfd, dstfd;
int dstlen, srclen, servlen, childPid;
struct SOCKS4_Request request;
struct RequestInfo requestInfo;
struct SOCKS4_Reply reply;
fd_set readSet, allReadSet, writeSet, allWriteSet;
int maxfd;
char buffer[BUF_MAX_SIZE];
int dstStatus = 0;
int srcStatus = 0;
int* bindPortOffset;

/*** shared-memory variables ***/
int shmid_bindPortOffset = -1;

/*** signal function declare ***/
void SigChild(int signo);

/*** function delcare ***/
void CreateConnection();
void NewClient();
int ReadRequest();
int FirewallRuleCheck();
int PassOrNot(char* rule);
void SOCKSConnect();
void SOCKSBind();
void SendReply(char result);
void Communication();
void Disconnect();

int main() {
    chdir(workDir);

    signal(SIGCHLD, SigChild);

    /*** set shared-memory ***/
    if( (shmid_bindPortOffset = shmget(SHMKEY, sizeof(int), PERMS|IPC_CREAT)) < 0) {
        printf("error: shmget failed (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
    }
    if( (bindPortOffset = (int *) shmat(shmid_bindPortOffset, (void *)0, 0)) == (void *)-1 ) {
        printf("error: shmat failed (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
    }
    *bindPortOffset = 0;

    stdoutfd = dup(STDOUT_FILENO);
    stderrfd = dup(STDERR_FILENO);
    CreateConnection();

    /*** when a connection comes ***/
    for(;;) {

        srclen = sizeof(src_addr);
        srcfd = accept(listenfd, (struct sockaddr*)&src_addr, &srclen);
        if(clientCount < LISTEN_MAX_NUM) {

            if(srcfd < 0) {
                printf("error: accept error (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
                close(srcfd);
                bzero(&src_addr, sizeof(src_addr));
                srclen = 0;
                srcfd = 0;
            }
            else {
                //printf("accept\n");fflush(stdout);

                NewClient();

                /*** fork a child process for client ***/
                if( (childPid = fork()) == -1) {
                    printf("error: fork() error (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
                    Disconnect();
                }
                else if(childPid == 0) {
                    /*** child process: client work environment ***/

                    close(listenfd);
                    listenfd = -1;
                    memset(&serv_addr, 0, sizeof(serv_addr));
                    servlen = -1;
                    dstfd = -1;
                    srcStatus = ACTIVE;

                    /*** set shared-memory ***/
                    if( (bindPortOffset = (int *) shmat(shmid_bindPortOffset, (void *)0, 0)) == (void *)-1 ) {
                        printf("error: shmat failed (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
                    }

                    /*** add srcfd to allReadSet, allWriteSet ***/
                    FD_SET(srcfd, &allReadSet);
                    FD_SET(srcfd, &allWriteSet);
                    if(srcfd > maxfd) maxfd = srcfd;

                    if(ReadRequest()) {
                        if(FirewallRuleCheck()) {
                            if(requestInfo.CD == CONNECT) SOCKSConnect();
                            else if(requestInfo.CD == BIND) SOCKSBind();
                            Communication();
                        }
                        else {
                            SendReply(REJECT);
                        }
                    }
                    else{
                        SendReply(REJECT);
                    }

                    Disconnect();
                    exit(0);
                }
                else {
                    /*** parent process ***/

                    close(srcfd);
                    bzero(&src_addr, sizeof(src_addr));
                    srclen = 0;
                    srcfd = 0;
                }
            }
        }
        else {
            printf("error: too many clients now (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
            close(srcfd);
            bzero(&src_addr, sizeof(src_addr));
            srclen = 0;
            srcfd = 0;
        }
    }
    return 0;
}

/*** SIGCHLD handler ***/
void SigChild(int signo) {
    if(childPid > 0) {
        --clientCount;
        printf("child process return (online: %d)\n", clientCount);fflush(stdout);
    }
    return;
}

/*** create listen socket and then bind, listen ***/
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
    ++clientCount;
    printf("new client (fd: %d, online: %d)\n", srcfd, clientCount);fflush(stdout);
    return;
}

/*** read the request from SOCKS client and return command number ***/
int ReadRequest() {
    bzero(&request, sizeof(request));
    read(srcfd, &request, sizeof(unsigned char) * 8);
    requestInfo.VN = request.VN;
    requestInfo.CD = request.CD;
    requestInfo.DST_PORT = (request.DST_PORT[0]<<8) + (request.DST_PORT[1]);
    sprintf(requestInfo.DST_IP, "%d.%d.%d.%d", request.DST_IP[0], request.DST_IP[1], request.DST_IP[2], request.DST_IP[3]);
    requestInfo.SRC_PORT = htons(src_addr.sin_port);
    strncpy(requestInfo.SRC_IP, inet_ntoa(src_addr.sin_addr), 16);

    unsigned char c;
    int i = 0;
    while(read(srcfd, &c, 1)) {
        if(c == '\0') break;
        if(i < USERID_MAX_LEN) request.USER_ID[i++] = c;
    }
    request.UNUSE = '\0';

    i = 0;
    int flag = fcntl(srcfd, F_GETFL, 0);
    if(fcntl(srcfd, F_SETFL, flag | O_NONBLOCK) < 0) {
        printf("error: fcntl add NONBLOCK flag failed (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
    }
    while(read(srcfd, &c, 1)) {
        if(c == '\0') break;
        if(i < DOMAIN_NAME_MAX_LEN) request.DOMAIN_NAME[i++] = c;
    }
    fcntl(srcfd, F_SETFL, flag);
    request.UNUSE2 = '\0';

    requestInfo.USER_ID = request.USER_ID;
    requestInfo.DOMAIN_NAME = request.DOMAIN_NAME;

    /*** request message ***/
    printf("VN: %d, CD: %d, DST IP: %s, DST PORT: %d, USERID: %s\n", (int)requestInfo.VN, (int)requestInfo.CD, requestInfo.DST_IP, requestInfo.DST_PORT, requestInfo.USER_ID);fflush(stdout);
    printf("Permit Src = %s(%d), Dst = %s(%d)\n", requestInfo.SRC_IP, requestInfo.SRC_PORT, requestInfo.DST_IP, requestInfo.DST_PORT);fflush(stdout);

    if(requestInfo.VN != 4) {
        printf("error: SOCKS version is not correct (VN: %d)\n", (int)requestInfo.VN);fflush(stdout);
        return 0;
    }
    else if(requestInfo.CD != CONNECT && requestInfo.CD != BIND) {
        printf("error: SOCKS command number is not correct (CD: %d)\n", (int)requestInfo.CD);fflush(stdout);
        return 0;
    }
    return (int)requestInfo.CD;
}

/*** read the socks configure file, check the firewall ruleset and return the result ***/
int FirewallRuleCheck() {
    int pass = 1, test = 0;
    char rule[RULE_MAX_LEN];
    char c;
    int i = 0;
    FILE* fp = fopen(socksConfig, "r");
    if(fp != NULL) {
        while( (c = fgetc(fp)) != EOF) {
            if(c == '\r') continue;
            else if(c == '\n' && (rule[0] == '#' || i == 0)) {
                memset(rule, 0, RULE_MAX_LEN);
                i = 0;
            }
            else if(c == '\n') {
                rule[i++] = '\0';
                test = PassOrNot(rule);
                if(test > 0) pass = 1;
                else if(test < 0) pass = 0;
                memset(rule, 0, RULE_MAX_LEN);
                i = 0;
                if(test == 2 || test == -2) break;
            }
            else rule[i++] = c;
        }
        fclose(fp);
    }
    return pass;
}

/*** check whether pass or not by the specific rule, return 0:not match, -1:deny, -2:force deny, 1:permit, 2:force permit ***/
int PassOrNot(char* rule) {
    printf("check rule: %s, ", rule);fflush(stdout);

    int regexFlags = REG_EXTENDED | REG_ICASE;
    regex_t preg;
    size_t nMatch = 1;
    regmatch_t pMatch[nMatch];
    char pattern[RULE_MAX_LEN];
    char* patternPtr = pattern;
    int result;

    int permit = 0;
    int force = 0;
    int pass = 0;
    char* rulePtr = strtok(rule, " ");
    if(strcmp(rulePtr, "PERMIT") == 0) permit = 1;
    else if(strcmp(rulePtr, "DENY") == 0) permit = 0;
    else {
        printf("error: invalid format <PERMIT or DENY>\n");fflush(stdout);
        return 0;
    }
    //printf("permit: %d, ", permit);fflush(stdout);

    if(rulePtr = strtok(NULL, " ")) {
        if(strcmp(rulePtr, "CONNECT") == 0) {
            if(requestInfo.CD != 1) {
                printf("\n");fflush(stdout);
                return 0;
            }
        }
        else if(strcmp(rulePtr, "BIND") == 0) {
            if(requestInfo.CD != 2) {
                printf("\n");fflush(stdout);
                return 0;
            }
        }
        else {
            printf("error: invalid format <CONNECT or BIND>\n");fflush(stdout);
            return 0;
        }
        //printf("%s, ", rulePtr);fflush(stdout);
    }
    else {
       printf("error: invalid format <CONNECT or BIND>\n");fflush(stdout);
       return 0;   
    }

    if(rulePtr = strtok(NULL, " ")) {
        memset(pattern, 0, RULE_MAX_LEN);
        strcat(pattern, "^");
        strcat(pattern, rulePtr);
        strcat(pattern, "$");
        if(regcomp(&preg, patternPtr, regexFlags) != 0) {
            /*** initialize Regex ***/
 
            printf("error: regex compile failed\n");fflush(stdout);
            return 0;
        }
        else if( (result = regexec(&preg, requestInfo.DST_IP, nMatch, pMatch, 0)) == 0) {
            /*** match ***/
 
            regfree(&preg);
            //printf("ip match, ");fflush(stdout);
        }
        else {
            /*** not match ***/
 
            regfree(&preg);
            printf("ip not match\n");fflush(stdout);
            return 0;
        }
    }
    else {
        printf("error: invalid format <dst ip regex string>\n");fflush(stdout);
        return 0;
    }

    if(rulePtr = strtok(NULL, " ")) {
        memset(pattern, 0, RULE_MAX_LEN);
        strcat(pattern, "^");
        strcat(pattern, rulePtr);
        strcat(pattern, "$");
        if(regcomp(&preg, patternPtr, regexFlags) != 0) {
            /*** initialize Regex ***/
 
            printf("error: regex compile failed\n");fflush(stdout);
            return 0;
        }
        else if( (result = regexec(&preg, requestInfo.DST_IP, nMatch, pMatch, 0)) == 0) {
            /*** match ***/
 
            regfree(&preg);
            //printf("port match");fflush(stdout);
        }
        else {
            /*** not match ***/
 
            regfree(&preg);
            printf("port not match\n");fflush(stdout);
            return 0;
        }
    }
    else {
        printf("error: invalid format <dst port regex string>\n");fflush(stdout);
        return 0;
    }

    if(rulePtr = strtok(NULL, " ")) {
        if(strcmp(rulePtr, "FORCE") == 0) {
            //printf("%s ", rulePtr);
            force = 1;
        }
    }

    if(permit == 1) pass = (1 + force);
    else if(permit == 0) pass = (-1 - force);

    printf("pass: %d\n", pass);fflush(stdout);
    return pass;
}

/*** SOCKS command connect, connect to dst host ***/
void SOCKSConnect() {
    int res = 0;
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_addr.s_addr = inet_addr(requestInfo.DST_IP);
    dst_addr.sin_port = htons(requestInfo.DST_PORT);
    if((dstfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("error: cannot open socket(errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
        SendReply(REJECT);
        return;
    }
    //printf("create socket for dst fd: %d\n", dstfd);fflush(stdout);

    /*** set non-blocking flag  ***/
    int flag = fcntl(dstfd, F_GETFL, 0);
    if(fcntl(dstfd, F_SETFL, flag | O_NONBLOCK) < 0) {
        printf("error: fcntl add NONBLOCK flag failed (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
    }
    //printf("set flag for non-blocking mode\n");fflush(stdout);

    res = connect(dstfd, (struct sockaddr*)&dst_addr, sizeof(dst_addr));
    if(res < 0 && errno != EINPROGRESS) {
        printf("error: connect to server: %s:%d failed (errno: %d - %s)\n", requestInfo.DST_IP, requestInfo.DST_PORT, errno, strerror(errno));fflush(stdout);
        SendReply(REJECT);
        return;
    }
    //printf("connect for dst fd: %d\n", dstfd);fflush(stdout);
    dstStatus = CONNECTING;
    
    /*** add dstfd to allReadSet, allWriteSet ***/
    FD_SET(dstfd, &allReadSet);
    FD_SET(dstfd, &allWriteSet);
    if(dstfd > maxfd) maxfd = dstfd;

    /*** send SOCKS reply to src host ***/
    SendReply(GRANT);
    return;
}

/*** SOCKS command bind, create listen socket and bind ***/
void SOCKSBind() {

    /*** create socket ***/
    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        printf("error: socket error (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
        SendReply(REJECT);
        return;
    }
    //printf("socket create for listen dst\n");fflush(stdout);

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*** select a port to bind ***/
    while(1) {
        *bindPortOffset = *bindPortOffset % BIND_MAX_NUM;
        serv_addr.sin_port = htons(BIND_TCP_PORT + *bindPortOffset);
        *bindPortOffset = *bindPortOffset + 1;

        /*** bind ***/
        if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            if(errno == EADDRINUSE) continue;
            else {
                printf("error: bind errorr (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
                SendReply(REJECT);
                return;
            }
        }
        //printf("bind for listen dst: localhost:%d\n", BIND_TCP_PORT + *bindPortOffset - 1);fflush(stdout);
        break;
    }

    /*** listen ***/
    listen(listenfd, LISTEN_MAX_NUM);
    //printf("listen for dst, fd: %d\n", listenfd);fflush(stdout);

    dstStatus = CONNECTING;

    /*** add listenfd to allReadSet ***/
    FD_SET(listenfd, &allReadSet);
    if(listenfd > maxfd) maxfd = listenfd;

    /*** send SOCKS reply to src host ***/
    SendReply(GRANT);
    return;
}

/*** send the reply to SOCKS client ***/
void SendReply(char result) {
    reply.VN = (unsigned char)0;
    reply.CD = (unsigned char)result;
    if(requestInfo.CD == CONNECT) {
        reply.DST_PORT[0] = request.DST_PORT[0];
        reply.DST_PORT[1] = request.DST_PORT[1];
        reply.DST_IP[0] = request.DST_IP[0];
        reply.DST_IP[1] = request.DST_IP[1];
        reply.DST_IP[2] = request.DST_IP[2];
        reply.DST_IP[3] = request.DST_IP[3];
    }
    else if(requestInfo.CD == BIND) {
        if(dstStatus == ACTIVE) {
            servlen = sizeof(serv_addr);
            getsockname(dstfd, (struct sockaddr*)&serv_addr, &servlen);
        }
        memcpy(reply.DST_PORT, &serv_addr.sin_port, sizeof(unsigned char) * 2);
        reply.DST_IP[0] = SOCKS_SERVER_IP[0];
        reply.DST_IP[1] = SOCKS_SERVER_IP[1];
        reply.DST_IP[2] = SOCKS_SERVER_IP[2];
        reply.DST_IP[3] = SOCKS_SERVER_IP[3];
    }
    write(srcfd, &reply, sizeof(reply));
    //printf("reply %d bytes\n", (int)sizeof(reply));fflush(stdout);

    /*** reply message ***/
    if(dstStatus == ACTIVE) return;
    if(requestInfo.CD == CONNECT) {
        printf("SOCKS_CONNECT ");fflush(stdout);
    }
    else if(requestInfo.CD == BIND) {
        printf("SOCKS_BIND ");fflush(stdout);
    }
    if(reply.CD == GRANT) {
        printf("GRANTED ....\n");fflush(stdout);
    }
    else if(reply.CD == REJECT) {
        printf("REJECTED ....\n");fflush(stdout);
    }
    return;
}

/*** infinite loop for src & dst communication ***/
void Communication() {
    while(1) {
        readSet = allReadSet;
        writeSet = allWriteSet;
        int readyNum = 0;
        int n = 0;
        if((readyNum = select(maxfd+1, &readSet, &writeSet, NULL, NULL)) < 0) {
            printf("error: select failed (readyNum: %d, errno: %d - %s)\n", readyNum, errno, strerror(errno));fflush(stdout);
            return;
        }
        else {
            //printf("readyNum: %d\n", readyNum);fflush(stdout);

            if(dstStatus == CONNECTING) {

                if(FD_ISSET(listenfd, &readSet)) {
                    /*** SOCKS BIND listen fd ***/

                    dstlen = sizeof(dst_addr);
                    dstfd = accept(listenfd, (struct sockaddr*)&dst_addr, &dstlen);

                    if(dstfd < 0) {
                        printf("error: accept error (errno: %d - %s)\n", errno, strerror(errno));fflush(stdout);
                    }
                    else {
                        //printf("accept dst connect\n");fflush(stdout);

                        /*** clear listenfd from allReadSet and add dstfd to allReadSet, allWriteSet ***/
                        FD_CLR(listenfd, &allReadSet);
                        FD_SET(dstfd, &allReadSet);
                        FD_SET(dstfd, &allWriteSet);
                        if(dstfd > maxfd) maxfd = dstfd;

                        close(listenfd);
                        listenfd = -1;
                        memset(&serv_addr, 0, sizeof(serv_addr));
                        servlen = -1;
                        dstStatus = ACTIVE;

                        SendReply(GRANT);
                        continue;
                    }
                }
                else if(FD_ISSET(dstfd, &readSet) || FD_ISSET(dstfd, &writeSet)) {
                    /*** SOCKS CONNECT connect fd ***/
 
                    /*** check non-blocking connect status ***/
                    int error = 0, errorLen = sizeof(error);
                    int res = 0;
 
                    res = getsockopt(dstfd, SOL_SOCKET, SO_ERROR, &error, &errorLen);
                    if(res < 0 || error != 0) {
                        printf("error: fd: %d connect to %s:%d failed (return:%d, error: %d, errno: %d - %s)\n", dstfd, requestInfo.DST_IP, requestInfo.DST_PORT, res, error, errno, strerror(errno));fflush(stdout);
                        return;
                    }
                    else {
                        printf("fd: %d connect to %s:%d successfully\n", dstfd, requestInfo.DST_IP, requestInfo.DST_PORT);fflush(stdout);
 
                        /*** non-blocking connect successfully, change status ***/
                        dstStatus = ACTIVE;
                        continue;
                    }
                }
            }
            else if(srcStatus == ACTIVE && dstStatus == ACTIVE) {
                if(FD_ISSET(srcfd, &readSet) && FD_ISSET(dstfd, &writeSet)) {
                    while((n = read(srcfd, buffer, BUF_MAX_SIZE)) > 0) {
                        printf("recieved %d bytes from src\n", n);fflush(stdout);
                        write(dstfd, buffer, n);
                        printf("send %d bytes to dst\n", n);fflush(stdout);
                        if(n < BUF_MAX_SIZE) break;
                    }
                    if(n == 0) break;
                }
                if(FD_ISSET(dstfd, &readSet) && FD_ISSET(srcfd, &writeSet)) {
                    while((n = read(dstfd, buffer, BUF_MAX_SIZE)) > 0) {
                        printf("recieved %d bytes from dst\n", n);fflush(stdout);
                        write(srcfd, buffer, n);
                        printf("send %d bytes to src\n", n);fflush(stdout);
                        if(n < BUF_MAX_SIZE) break;
                    }
                    if(n == 0) break;
                }
            }
        }
    }
    return;
}

/*** close the connection of the current client ***/
void Disconnect() {
    if(srcfd != -1) {
        close(srcfd);
        //printf("client (fd: %d) left\n", srcfd);fflush(stdout);
    }
    if(dstfd != -1) {
        close(dstfd);
        //printf("dst connection (fd: %d) close\n", dstfd);fflush(stdout);
    }
    bzero(&src_addr, sizeof(src_addr));
    bzero(&dst_addr, sizeof(dst_addr));
    srclen = 0;
    dstlen = 0;
    srcfd = -1;
    dstfd = -1;
    bzero(buffer, BUF_MAX_SIZE);
    --clientCount;
    return;
}
