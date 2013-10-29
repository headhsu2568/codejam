/******
 *
 * 'NP hw1 simple server practice'
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

/*** Define value ***/
#define PATH_MAX_NUM 100
#define PATH_MAX_LEN 1000
#define CMD_MAX_LEN 100
#define BUF_MAX_SIZE 2000
#define CMD_MAX_NUM 100
#define PIPE_MAX_NUM 1000
#define SERV_TCP_PORT 7777

/*** function delcare ***/
int ReadCmdLine(int sockfd, char* buf, int* n);

int main(int argc, char** argv) {
    int n, childPid;
    char buf[BUF_MAX_SIZE];
    char execPath[PATH_MAX_LEN];
    char execCmd[CMD_MAX_LEN];
    int cmdLen = 0;
    char* execArgv[BUF_MAX_SIZE];
    int argvNum = 0;
    int cmdNum = 0;

    int sockfd, newsockfd, clilen;
    struct sockaddr_in cli_addr, serv_addr;

    if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        write(STDERR_FILENO, "socket error!\n", 14);
        exit(1);
    }
    write(STDOUT_FILENO, "socket create\n", 14);

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        write(STDERR_FILENO, "bind error!\n", 12);
        exit(1);
    }
    write(STDOUT_FILENO, "bind\n", 5);

    listen(sockfd, 1);
    write(STDOUT_FILENO, "listen\n", 7);

    for(;;) {

        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
        if(newsockfd < 0) {
            write(STDERR_FILENO, "accept error!\n", 14);
            exit(1);
        }
        write(STDOUT_FILENO, "accept\n", 7);

        while(1) {

            /*** clean & initialize variables ***/
            bzero(buf, BUF_MAX_SIZE);
            bzero(execPath, PATH_MAX_LEN);
            bzero(execCmd, CMD_MAX_LEN);
            bzero(execArgv, sizeof(execArgv));
            cmdNum = 0;
            argvNum = 0;
            cmdLen = 0;

            if( ReadCmdLine(newsockfd, buf, &n) > 0) {
                write(STDOUT_FILENO, buf, n);
            }
        }

        close(newsockfd);
        write(STDOUT_FILENO, "client close\n", 13);
    }

    close(sockfd);
    write(STDOUT_FILENO, "server close\n", 13);

    return 0;
}

/*** read a command line to buffer ***/
int ReadCmdLine(int sockfd, char* buf, int* n) {
    /*
    if( (*n = read(0, buf, BUF_MAX_SIZE)) < 1) {
        write(STDERR_FILENO, "read error!\n", 12);
    }
    else {
        printf("read %d bytes\n", *n);
    }
    */

    int i = 1, result;
    char c;
    for(; i < BUF_MAX_SIZE-1; ++i) {
        if( (result = read(sockfd, &c, 1)) == 1) {
            *buf = c;
            buf = buf + 1;
            if(c == '\n') {
                break;
            }
        }
        else if (result == 0){
            *buf = '\n';
            buf = buf + 1;
            break;
        }
        else {
            write(STDERR_FILENO, "read error!\n", 12);
            return -1;
        }
    }
    *buf = '\0';
    *n = i;
    printf("read %d bytes\n", *n);
    return i;
}

