/******
 *
 * 'NP hw3 part1 - CGI program'
 * by headhsu
 *
 * gcc -fno-stack-protector -lfcgi -o cgi_hw3.fcgi cgi_hw3.c
 *
 ******/

#include "/usr/include/fcgi_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

/*** Define value ***/
#define BUF_MAX_SIZE 20480
#define PARAM_MAX_NUM 30
#define QRYSTR_MAX_LEN 1024
#define BATCHFILE_NAME_MAX_LEN 20

/*** Define connection status value ***/
#define CONNECTING 1
#define READING 2
#define WRITING 3
#define DONE 4

/*** data structure definition ***/
struct PARAM {
    char* paramKey;
    char* paramValue;
};
struct CONN {
    struct sockaddr_in conn_addr;
    int connfd;
    int status;
    char* no;
    char* ip;
    char* port;
    char* batchFile;
    FILE* fp;
};

/*** environment parameters ***/
char buffer[BUF_MAX_SIZE];
char buffer2[BUF_MAX_SIZE];
int requestCount = 0;
char* WorkDir = "/var/www/nphw3";

/*** connection variables ***/
int maxfd, readyNum;
fd_set readSet, writeSet, allReadSet, allWriteSet;
struct CONN conn[PARAM_MAX_NUM];
int connNum = 0;
int activeConnNum = 0;

/*** request variables ***/
struct PARAM param[PARAM_MAX_NUM];
int paramNum = 0;
char queryString[QRYSTR_MAX_LEN];
char queryStringBak[QRYSTR_MAX_LEN];
int queryStringLen = 0;

/*** function declare ***/
void Initialization();
void ParseParameter();
void SetConnVar();
void CreateConnectionForConn();
void DisconnectConn(int i);
void ReadFromConn(int i);
void WriteToConn(int i);
void ConnectionEnd();
void ListAllConnVar();

int main() {
    int i = 0;
    
    chdir(WorkDir);

    /*** when http cgi request come ***/
    while(FCGI_Accept() >= 0) {
        
        Initialization();

        ParseParameter();

        SetConnVar();

        CreateConnectionForConn();

        readSet = allReadSet;
        writeSet = allWriteSet;

        /*** select loop ***/
        while(activeConnNum > 0) {
            readSet = allReadSet;
            writeSet = allWriteSet;

            if( (readyNum = select(maxfd+1, &readSet, &writeSet, NULL, NULL)) < 0) {
                printf("error: select failed<br>\n");fflush(stdout);
                continue;
            }
            //printf("readyNum: %d, connNum: %d<br>\n", readyNum, connNum);fflush(stdout);

            for(i = 0; i < connNum; ++i) {

                if(conn[i].connfd < 0) continue;

                if(conn[i].status == CONNECTING && ((FD_ISSET(conn[i].connfd, &readSet) || FD_ISSET(conn[i].connfd, &writeSet)))) {

                    /*** check non-blocking connect status ***/
                    int error = 0, errorLen = sizeof(error);
                    int res = 0;

                    res = getsockopt(conn[i].connfd, SOL_SOCKET, SO_ERROR, &error, &errorLen);
                    if(res < 0 || error != 0) {
                        printf("error: fd: %d connection to %s:%s failed (return:%d, error: %d, errno: %d)<br>\n",conn[i].connfd, conn[i].ip, conn[i].port, res, error, errno);fflush(stdout);
                        DisconnectConn(i);
                    }
                    else {
                        //printf("fd: %d connect %s:%s successfully<br>\n",conn[i].connfd, conn[i].ip, conn[i].port);fflush(stdout);

                        /*** non-blocking connect successfully, change status ***/
                        conn[i].status = READING;
                        FD_CLR(conn[i].connfd, &writeSet);
                    }

                    /*** short cut if no more readable fd is ready ***/
                    //if(--readyNum < 1) break;
                }
                else if(conn[i].status == WRITING && FD_ISSET(conn[i].connfd, &writeSet)) {
                    //printf("write to fd: %d<br>\n", conn[i].connfd);fflush(stdout);

                    WriteToConn(i);

                    /*** short cut if no more readable fd is ready ***/
                    //if(--readyNum < 1) break;
                }
                else if(conn[i].status == READING && FD_ISSET(conn[i].connfd, &readSet)) {
                    //printf("read from fd: %d<br>\n", conn[i].connfd);fflush(stdout);

                    ReadFromConn(i);

                    /*** short cut if no more readable fd is ready ***/
                    //if(--readyNum < 1) break;
                }
            }
        }

        ConnectionEnd();

        ++requestCount;
        if(requestCount > 100) break;
    }

    FCGI_Finish();
}

/*** when new client come, initial all global variables to default ***/
void Initialization() {
    bzero(buffer, sizeof(buffer));
    maxfd = 0;
    readyNum = 0;
    FD_ZERO(&allReadSet);
    FD_ZERO(&allWriteSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&readSet);
    bzero(conn, sizeof(conn));
    connNum = 0;
    activeConnNum = 0;
    bzero(param, sizeof(param));
    paramNum = 0;
    bzero(queryString, sizeof(queryString));
    bzero(queryStringBak, sizeof(queryStringBak));
    queryStringLen = 0;

    printf("Content-type: text/html\nStatus: 200 OK\n\n");fflush(stdout);
    printf("<html>\n");fflush(stdout);
    printf("<head>\n");fflush(stdout);
    printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />\n");fflush(stdout);
    printf("<title>NP homework3 part1 - CGI by ychsu</title>\n");fflush(stdout);
    printf("<h1>NP homework3 part1 - CGI by ychsu</h1>\n");fflush(stdout);
    printf("</head>\n");fflush(stdout);
    printf("<body bgcolor=#336699>\n");fflush(stdout);
    printf("<br>Request number %d running on host <i>%s</i><br>\n", requestCount, getenv("SERVER_NAME"));fflush(stdout);

    return;
}

/*** QUERY_STRING(GET parameters) parser ***/
void ParseParameter() {
    int queryStringLen = 0;
    int i = 0;
    if(strlen(getenv("QUERY_STRING")) > 0) {
        strncpy(queryString, getenv("QUERY_STRING"), QRYSTR_MAX_LEN);
        queryStringLen = strlen(queryString);
        strncpy(queryStringBak, queryString, queryStringLen);
        
        char* ptrTmp = queryString;
        if( (ptrTmp = strtok(ptrTmp, "&")) ) {
            param[i].paramKey = ptrTmp;
            ++i;
            for(; (ptrTmp = strtok(NULL, "&")); ++i) {
                param[i].paramKey = ptrTmp;
            }
            paramNum = i;
        }

        ptrTmp = NULL;
        for(i = 0; i < paramNum; ++i) {
            ptrTmp = param[i].paramKey;
            if( (ptrTmp = strpbrk(ptrTmp, "=")) ) {
                strtok(param[i].paramKey, "=");
                param[i].paramValue = ptrTmp + 1;
            }
            else {
                printf("parse GET parameter error!<br>\n");fflush(stdout);
            }
            //printf("%s : %s<br>\n", param[i].paramKey, param[i].paramValue);fflush(stdout);
        }
    }
    else {
        strcpy(queryString, "(null)");
        strcpy(queryStringBak, "(null)");
    }
    //printf("Request has query string with %s<br>\n", queryStringBak);fflush(stdout);
    return;
}

/*** set variables for each conn (an array of struct CONN) ***/
void SetConnVar() {
    int i = 0, j = 0;
    connNum = 0;
    for(i = 0; i < paramNum; ++i) {
        if(strlen(param[i].paramValue) < 1) continue;
        if(*(param[i].paramKey) == 'h') {
            /*** set no. and ip ***/

            for(j = 0; j < connNum; ++j) {
                if(*(param[i].paramKey + 1) == *(conn[j].no)) {
                    conn[j].ip = param[i].paramValue;
                    break;
                }
            }
            if(j == connNum) {
                conn[connNum].no = param[i].paramKey + 1;
                conn[connNum].ip = param[i].paramValue;
                conn[connNum].fp = NULL;
                ++connNum;
            }
        }
        else if(*(param[i].paramKey) == 'p') {
            /*** set no. and port ***/

            for(j = 0; j < connNum; ++j) {
                if(*(param[i].paramKey + 1) == *(conn[j].no)) {
                    conn[j].port = param[i].paramValue;
                    break;
                }
            }
            if(j == connNum) {
                conn[connNum].no = param[i].paramKey + 1;
                conn[connNum].port = param[i].paramValue;
                conn[connNum].fp = NULL;
                ++connNum;
            }
        }
        else if(*(param[i].paramKey) == 'f') {
            /*** set no. and batch file name ***/

            for(j = 0; j < connNum; ++j) {
                if(*(param[i].paramKey + 1) == *(conn[j].no)) {
                    conn[j].batchFile = param[i].paramValue;
                    if(conn[j].fp = fopen(conn[j].batchFile, "r")) {
                        //printf("open file: %s<br>\n", conn[j].batchFile);fflush(stdout);
                    }
                    else {
                        conn[j].fp = NULL;
                        printf("error: open file %s failed (errno: %d-%s)<br>\n", conn[j].batchFile, errno, strerror(errno));fflush(stdout);
                    }
                    break;
                }
            }
            if(j == connNum) {
                conn[connNum].no = param[i].paramKey + 1;
                conn[connNum].batchFile = param[i].paramValue;
                if(conn[connNum].fp = fopen(conn[connNum].batchFile, "r")) {
                    //printf("open file: %s<br>\n", conn[connNum].batchFile);fflush(stdout);
                }
                else {
                    conn[connNum].fp = NULL;
                    printf("error: open file %s failed<br>\n", conn[connNum].batchFile);fflush(stdout);
                }
                ++connNum;
            }
        }
    }
    //ListAllConnVar();
    return;
}

/*** after setting each conn variables, create these connection to specific server ***/
void CreateConnectionForConn() {
    int i = 0, res = 0;

    /*** html table thead ***/
    //printf("<font face='Courier New' size=2 color=#FFFF99>\n");fflush(stdout);
    printf("<table width=\"1024\" border=\"1\">\n");fflush(stdout);
    printf("<tr>\n");fflush(stdout);
    for(i = 0; i < connNum; ++i) {
        conn[i].conn_addr.sin_family = AF_INET;
        conn[i].conn_addr.sin_addr.s_addr = inet_addr(conn[i].ip);
        conn[i].conn_addr.sin_port = htons(atoi(conn[i].port));
        if((conn[i].connfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
            printf("error: cannot open socket!<br>\n");fflush(stdout);
            continue;
        }
        //printf("create socket fd: %d<br>\n", conn[i].connfd);fflush(stdout);

        /*** set non-blocking flag  ***/
        int flag = fcntl(conn[i].connfd, F_GETFL, 0);
        if(fcntl(conn[i].connfd, F_SETFL, flag | O_NONBLOCK) < 0) {
            close(conn[i].connfd);
            conn[i].connfd = -1;
            continue;
        }
        //printf("set flag for non-blocking mode<br>\n");fflush(stdout);

        res = connect(conn[i].connfd, (struct sockaddr*)&conn[i].conn_addr, sizeof(conn[i].conn_addr));
        if(res < 0 && errno!=EINPROGRESS) {
            printf("error: connect to server: %s:%s failed<br>\n", conn[i].ip, conn[i].port);fflush(stdout);
            continue;
        }
        //printf("connect to server: %s:%s!<br>\n", conn[i].ip, conn[i].port);fflush(stdout);
        conn[i].status = CONNECTING;
        ++activeConnNum;

        /*** add fd to allReadSet and allWriteSet ***/
        FD_SET(conn[i].connfd, &allReadSet);
        FD_SET(conn[i].connfd, &allWriteSet);

        /*** set maxfd if needed ***/
        if(conn[i].connfd > maxfd) maxfd = conn[i].connfd;

        printf("<td>%s</td>\n", conn[i].ip);fflush(stdout);
    }
    printf("</tr>\n");fflush(stdout);


    /*** html table tbody ***/
    printf("<tr>\n");fflush(stdout);
    for(i = 0 ; i < connNum; ++i) {
        printf("<td valign=\"top\" id=\"m%c\"></td>\n", *(conn[i].no));fflush(stdout);
    }
    printf("</tr>\n");fflush(stdout);
    printf("</table>\n");fflush(stdout);
    printf("<font face=\"Courier New\" size=3 color=#FFFF99>\n");fflush(stdout);
    return;
}

/*** disconnet the specific connection ***/
void DisconnectConn(int i) {
    conn[i].status = DONE;
    if(conn[i].connfd != -1) {
        FD_CLR(conn[i].connfd, &allReadSet);
        FD_CLR(conn[i].connfd, &allWriteSet);
        close(conn[i].connfd);
        conn[i].connfd = -1;
        if(conn[i].fp != NULL) fclose(conn[i].fp);
        conn[i].fp = NULL;
        --activeConnNum;
    }
    return;
}

/*** read messages from the specific connection ***/
void ReadFromConn(int i) {
    int n = 0, res = 0, changeOption = 0;
    char c;
    while( (res = read(conn[i].connfd, &c, 1)) > 0) {
        //printf("read: %c (%d)<br>", c, c);fflush(stdout);
        if(c == '\r') continue;

        /*** set changeOption flag ***/
        if(c == '%' && changeOption == 0) {
            changeOption = 1;
        }
        else if(c == ' ' && changeOption == 1) {
            changeOption = 2;
        }
        else if(c == '\n') {
            changeOption = 0;
        }
        else if(changeOption != 2) {
            changeOption = -1;
        }
        
        /*** handle the read character ***/
        if(c == '\n') {
            buffer[n] = '\0';
            printf("<script>document.all['m%c'].innerHTML += \"%s<br>\";</script>\n", *conn[i].no, buffer);fflush(stdout);
            bzero(buffer, BUF_MAX_SIZE);
            n = 0;
        }
        else if(n == BUF_MAX_SIZE - 3) {
            buffer[n++] = c;
            buffer[n] = '\0';
            printf("<script>document.all['m%c'].innerHTML += \"%s\";</script>\n", *conn[i].no, buffer);fflush(stdout);
            bzero(buffer, BUF_MAX_SIZE);
            n = 0;
            //printf("read overflow!<br>\n");fflush(stdout);
        }
        else {
            if(c == '\"') buffer[n++] = '\\';
            buffer[n++] = c;
            if(n == BUF_MAX_SIZE - 3) {
                buffer[n++] = c;
                buffer[n] = '\0';
                printf("<script>document.all['m%c'].innerHTML += \"%s\";</script>\n", *conn[i].no, buffer);fflush(stdout);
                bzero(buffer, BUF_MAX_SIZE);
                n = 0;
                //printf("read overflow!<br>\n");fflush(stdout);
            }
        }

        if(changeOption == 2) break;
    }
    if(n > 0) {
        buffer[n] = '\0';
        printf("<script>document.all['m%c'].innerHTML += \"%s\";</script>\n", *conn[i].no, buffer);fflush(stdout);
        bzero(buffer, BUF_MAX_SIZE);
    }
    if(res == 0) {
        DisconnectConn(i);
        return;
    }

    /*** if read the '%' character, change the status to WRITING that can write a command line to the connection ***/
    if(changeOption == 2) {
        //printf("change fd: %d to WRITING status<br>\n", conn[i].connfd);
        conn[i].status = WRITING;
    }
    else conn[i].status = READING;

    return;
}

/*** write a command line from the specific file to the specific connection  ***/
void WriteToConn(int i) {
    int j = 0, j2 = 0;
    char c;

    /*** the pipe type: <<EOF finite state machine (
     * 0:(ready for read the first '<'), 
     * 1:<, 
     * 2:<<, 
     * 3:<<E, 
     * 4:<<EO, 
     * 5:<<EOF, 
     * 6:<<EOF\n, 
     * 7: (ready for read 'E')
     * 8:E, 
     * 9:EO, 
     * 10:EOF) ***/
    int pipeEofFSM = 0;

    if(conn[i].fp != NULL) {
        while( (c = fgetc(conn[i].fp)) != EOF) {
            //write(STDOUT_FILENO, &c, 1);
            //printf("<script>document.all['m%c'].innerHTML += \"(%d)\";</script>\n", *conn[i].no, c);fflush(stdout);

            if(c == '\r') continue;
            else if(c == '\n' && (pipeEofFSM < 5 || pipeEofFSM == 10)) {
                /*** write when read a '\n' character ***/

                pipeEofFSM = 0;
                buffer2[j2] = '\0';
                printf("<script>document.all['m%c'].innerHTML += \"<b>%s</b><br>\";</script>\n", *conn[i].no, buffer2);fflush(stdout);

                buffer[j] = '\n';
                buffer[j+1] = '\0';
                write(conn[i].connfd, buffer, strlen(buffer));
                bzero(buffer, BUF_MAX_SIZE);
                bzero(buffer2, BUF_MAX_SIZE);
                break;
            }
            else if(c == '\n' && pipeEofFSM >= 5) {
                /*** write when read a '\n' character and then keep reading until read "EOF\n" ***/

                pipeEofFSM = 7;
                buffer2[j2] = '\0';
                printf("<script>document.all['m%c'].innerHTML += \"<b>%s</b><br>\";</script>\n", *conn[i].no, buffer2);fflush(stdout);

                buffer[j] = '\n';
                buffer[j+1] = '\0';
                write(conn[i].connfd, buffer, strlen(buffer));
                bzero(buffer, BUF_MAX_SIZE);
                bzero(buffer2, BUF_MAX_SIZE);
                j = 0;
                j2 = 0;
                continue;
            }
            else if(c == '<' && (pipeEofFSM == 0 || pipeEofFSM == 1)) ++pipeEofFSM;
            else if(c == 'E' && (pipeEofFSM == 2 || pipeEofFSM == 7)) ++pipeEofFSM;
            else if(c == 'O' && (pipeEofFSM == 3 || pipeEofFSM == 8)) ++pipeEofFSM;
            else if(c == 'F' && (pipeEofFSM == 4 || pipeEofFSM == 9)) ++pipeEofFSM;
            else if(c == ' ' && pipeEofFSM == 2) pipeEofFSM = 2;
            else if(c == ' ' && pipeEofFSM == -1) pipeEofFSM = 0;
            else if(pipeEofFSM >= 6) pipeEofFSM = 6;
            else pipeEofFSM = -1;

            /*** handle the html encode ***/
            if(c == '\"') {
                buffer2[j2++] = '&';
                buffer2[j2++] = 'q';
                buffer2[j2++] = 'u';
                buffer2[j2++] = 'a';
                buffer2[j2++] = 't';
                buffer2[j2++] = ';';
            }
            else if(c == '<') {
                buffer2[j2++] = '&';
                buffer2[j2++] = 'l';
                buffer2[j2++] = 't';
                buffer2[j2++] = ';';
            }
            else if(c == '>') {
                buffer2[j2++] = '&';
                buffer2[j2++] = 'g';
                buffer2[j2++] = 't';
                buffer2[j2++] = ';';
            }
            else if(c == '&') {
                buffer2[j2++] = '&';
                buffer2[j2++] = 'a';
                buffer2[j2++] = 'm';
                buffer2[j2++] = 'p';
                buffer2[j2++] = ';';
            }
            else if(c == ' ') {
                buffer2[j2++] = '&';
                buffer2[j2++] = 'n';
                buffer2[j2++] = 'b';
                buffer2[j2++] = 's';
                buffer2[j2++] = 'p';
                buffer2[j2++] = ';';
            }
            else buffer2[j2++] = c;

            buffer[j++] = c;

            /*** write when buffer is full and then keep read ***/
            if(j2 >= BUF_MAX_SIZE - 7) {
                buffer2[j2] = '\0';
                printf("<script>document.all['m%c'].innerHTML += \"<b>%s</b>\";</script>\n", *conn[i].no, buffer2);fflush(stdout);
                //printf("write overflow!<br>\n");fflush(stdout);

                buffer[j] = '\0';
                write(conn[i].connfd, buffer, strlen(buffer));
                j = 0;
                j2 = 0;
                bzero(buffer, BUF_MAX_SIZE);
                bzero(buffer2, BUF_MAX_SIZE);
            }
        }
 
        /*** close the batch file ***/
        if(c == EOF) {
            fclose(conn[i].fp);
            conn[i].fp = NULL;
            DisconnectConn(i);
            //printf("end of file %s<br>\n", conn[i].batchFile);fflush(stdout);
            return;
        }

        /*** after write to connection, change the status to READING that can wait for response from connection  ***/
        //printf("change fd: %d to READING status<br>\n", conn[i].connfd);fflush(stdout);
        conn[i].status = READING;
    }
    else {
        DisconnectConn(i);
        printf("error: nothing to write<br>\n");fflush(stdout);
        return;
    }

    return;
}

/*** end connections ***/
void ConnectionEnd() {
    int i = 0;
    for(; i < connNum; ++i) {
        DisconnectConn(i);
    }

    //printf("</font>\n");fflush(stdout);
    //printf("</body>\n");fflush(stdout);
    //printf("</html>\n");fflush(stdout);

    bzero(buffer, sizeof(buffer));
    maxfd = 0;
    readyNum = 0;
    FD_ZERO(&allReadSet);
    FD_ZERO(&allWriteSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&readSet);
    bzero(conn, sizeof(conn));
    connNum = 0;
    activeConnNum = 0;
    bzero(param, sizeof(param));
    paramNum = 0;
    bzero(queryString, sizeof(queryString));
    bzero(queryStringBak, sizeof(queryStringBak));
    queryStringLen = 0;

    return;
}

void ListAllConnVar() {
    int i = 0;
    for(; i < connNum; ++i) {
        printf("server %d (fd:%d)[", i + 1, conn[i].connfd);fflush(stdout);
        printf("ip(h%c): %s, ", *(conn[i].no), conn[i].ip);fflush(stdout);
        printf("port(p%c): %s, ", *(conn[i].no), conn[i].port);fflush(stdout);
        printf("batch file(f%c): %s.]<br>\n",  *(conn[i].no), conn[i].batchFile);fflush(stdout);
    }
    return;
}
