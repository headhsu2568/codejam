/******
 *
 * 'simple single-process echo server'
 * by headhsu
 *
 ******/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*** Define value ***/
#define SERV_TCP_PORT 7777
#define MAX_LISTEN_NUM 50
#define FD_SET_SIZE 100
#define MAX_BUF_SIZE 20000

/*** connection variables ***/
struct sockaddr_in cli_addr, serv_addr;
int listenfd, connfd, sockfd;
int clilen;
int clientfd[FD_SET_SIZE];
int maxfd, maxi, readyNum;
fd_set readSet, allSet;

/*** function delcare ***/
void CreateConnection();

int main() {
    int n = 0, i = 0;
    char buf[MAX_BUF_SIZE];

    CreateConnection();

    /*** fd infinite loop ***/
    for(;;) {
        readSet = allSet;
        readyNum = select(maxfd + 1, &readSet, NULL, NULL, NULL);

        /*** when a connection comes ***/
        if(FD_ISSET(listenfd, &readSet)) {

            /*** accept ***/
            clilen = sizeof(cli_addr);
            connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
            if(connfd < 0) {
                write(STDERR_FILENO, "accept error!\n", 14);
            }
            else {
                write(STDERR_FILENO, "accept\n", 7);

                /*** record the fd to client fd array & set fd related variables ***/
                for(i = 0; i < FD_SET_SIZE; ++i) {
                    if(clientfd[i] < 0) {
                        clientfd[i] = connfd;
                        write(connfd, "Hi, client.\n% ", 14);
                        unsigned int client_port = htons(cli_addr.sin_port);
                        char* client_ip = inet_ntoa(cli_addr.sin_addr);
                        printf("client %2d (from %s:%d) come in\n", i, client_ip, client_port);fflush(stdout);
                        break;
                    }
                }
                if(i == FD_SET_SIZE) {
                    write(STDERR_FILENO, "error: too many clients now\n", 28);
                    close(connfd);
                }
                else {

                    /*** add clientfd to allSet ***/
                    FD_SET(connfd, &allSet);

                    /*** set maxfd & maxi if needed ***/
                    if(connfd > maxfd) maxfd = connfd;
                    if(i > maxi) maxi = i;
                }
            }

            /*** short cut if no more readable fd is ready ***/
            if(--readyNum < 1) continue;
        }

        /*** readable client fd loop ***/
        for(i = 0; i <= maxi; ++i) {
            sockfd = clientfd[i];
            if(sockfd < 0) continue;

            if(FD_ISSET(sockfd, &readSet)) {
                bzero(buf, MAX_BUF_SIZE);
                n = read(sockfd, buf, MAX_BUF_SIZE);
                if(n == 0) {

                    /*** connection close from client ***/
                    close(sockfd);
                    FD_CLR(sockfd, &allSet);
                    clientfd[i] = -1;
                }
                else if(n > 0) {
                    write(sockfd, buf, n);
                    write(sockfd, "% ", 2);
                }
                else {
                    write(STDERR_FILENO, "error: read error!\n", 19);
                }

                /*** short cut if no more readable fd is ready ***/
                if(--readyNum < 1) break;
            }
        }
    }
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

    /** initialize max value, client fd and fd set ****/
    maxfd = listenfd;
    maxi = -1;
    for(i = 0; i < FD_SET_SIZE; ++i) {
        clientfd[i] = -1;
    }
    FD_ZERO(&allSet);
    FD_SET(listenfd, &allSet);

    return;
}

