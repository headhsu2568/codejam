/******
 *
 * 'simple connection-oriented echo server'
 * by headhsu
 *
 ******/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

/*** Define value ***/
#define MAX_LISTEN_NUM 50
#define FD_SET_SIZE 100
#define MAX_BUF_SIZE 20000
#define SERV_TCP_PORT 7778

#define SEMKEY ((key_t) 7890)
#define SEMKEY1 ((key_t) 7891)
#define SEMKEY2 ((key_t) 7892)
#define PERMS 0666
#define BIGCOUNT 99

#define SHM_SIZE 4096
#define MAX_MESGDATA_SIZE (4096-16)
#define MESG_HEADER_SIZE (sizeof(Mesg) - MAX_MESGDATA_SIZE)

/*** data structure definition ***/
struct ClientInfo {

    /*** connection variables ***/
    int fd;
    int clilen;
    struct sockaddr_in cli_addr;
    unsigned int port;
    char ip[16];

    /*** user environment ***/
    char nickname[21];
};
struct Mesg {
    int mesg_len;
    long mesg_type;
    char mesg_data[MAX_MESGDATA_SIZE];
};

/***
 * struct sembuf {
 *      ushort sem_num;
 *      short sem_op;
 *      short sem_flg;
 * };
***/
static struct sembuf op_lock[2] = {
    2, 0, 0,
    2, 1, SEM_UNDO
};
static struct sembuf op_unlock[1] = {
    2, -1, SEM_UNDO
};
static struct sembuf op_endcreate[2] = {
    1, -1, SEM_UNDO,
    2, -1, SEM_UNDO
};
static struct sembuf op_open[1] = {
    1, -1, SEM_UNDO
};
static struct sembuf op_close[3] = {
    2, 0, 0,
    2, 1, SEM_UNDO,
    1, 1, SEM_UNDO
};
static struct sembuf op_op[1] = {
    0, BIGCOUNT, SEM_UNDO
};
union semun {
    int val;
    struct semid_ds *buff;
    ushort *array;
} semctl_arg;

/*** environment parameters ***/
int stdoutfd = STDOUT_FILENO;
int stderrfd = STDERR_FILENO;
struct ClientInfo client[MAX_LISTEN_NUM];
int curClientNo = -1;

/*** connection variables ***/
struct sockaddr_in cli_addr, serv_addr;
int listenfd, connfd, sockfd;
int childPid;

/*** shm & sem variables ***/
int shmid, clisem, servsem;
struct Mesg* mesgptr;

/*** function declare ***/
void CreateConnection();
void NewClient();
void ClientShell(int clientfd);
void Exit();

/*** semaphore function declare ***/
int sem_create(key_t key, int initval);
int sem_open(key_t key);
void sem_rm(int id);
void sem_close(int id);
void sem_op(int id, int value);

int main() {
    int n = 0, i = 0;
    char buf[MAX_BUF_SIZE];

    CreateConnection();
    stdoutfd = dup(STDOUT_FILENO);
    stderrfd = dup(STDERR_FILENO);

    while(1) {
        
        /*** accept ***/
        for(i = 0; i < MAX_LISTEN_NUM; ++i) {
            if(client[i].fd < 0) {
                client[i].clilen = sizeof(client[i].cli_addr);
                client[i].fd = accept(listenfd, (struct sockaddr*)&client[i].cli_addr, &client[i].clilen);
                if(client[i].fd < 0) {
                    write(STDERR_FILENO, "accept error!\n", 14);
                }
                else {
                    write(STDERR_FILENO, "accept\n", 7);
                    curClientNo = i;
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
            break;
        }

        /*** fork child process for the client ***/
        if( (childPid = fork()) == -1) {
            write(STDERR_FILENO, "fork() error.\n", 14);
            Exit(curClientNo);
        }
        else if(childPid == 0) {
            /*** child process: client work environment ***/

            close(listenfd);
            ClientShell(curClientNo);
            Exit(curClientNo);
            exit(1);
        }
        else {
            close(client[curClientNo].fd);
        }
    }

    if(childPid > 0) {
        wait();
    }

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

    /** initialize max value, client fd and fd set ****/
    for(i = 0; i < MAX_LISTEN_NUM; ++i) {
        client[i].fd = -1;
        client[i].clilen = 0;
        client[i].port = 0;
    }
    return;
}

/*** initialize a new client after accepting a connection ***/
void NewClient() {

    /*** initialize client (struct ClientInfo) ***/
    client[curClientNo].port = htons(client[curClientNo].cli_addr.sin_port);
    strncpy(client[curClientNo].ip, inet_ntoa(client[curClientNo].cli_addr.sin_addr), 16);
    strcpy(client[curClientNo].nickname, "(No Name)");

    printf("new client #%d\n", curClientNo+1);fflush(stdout);

    return;
}

void ClientShell(int clientNo) {
    dup2(client[clientNo].fd, STDOUT_FILENO);
    dup2(client[clientNo].fd, STDERR_FILENO);
    write(STDOUT_FILENO, "Welcome.\n", 9);
    sleep(1);
    write(STDOUT_FILENO, "6.\n", 3);
    sleep(1);
    write(STDOUT_FILENO, "5.\n", 3);
    sleep(1);
    write(STDOUT_FILENO, "4.\n", 3);
    sleep(1);
    write(STDOUT_FILENO, "3.\n", 3);
    sleep(1);
    write(STDOUT_FILENO, "2.\n", 3);
    sleep(1);
    write(STDOUT_FILENO, "1.\n", 3);
    sleep(1);
    write(STDOUT_FILENO, "bye!.\n", 6);
    return;
}

/*** exit ***/
void Exit(int clientNo) {

    /*** close client connection ***/
    close(client[clientNo].fd);

    /*** reset client[clientNo] to default values ***/
    client[curClientNo].fd = -1;
    client[curClientNo].clilen = 0;
    bzero(&client[curClientNo].cli_addr, sizeof(struct sockaddr_in));
    client[curClientNo].port = 0;
    bzero(client[curClientNo].ip, 16);
    bzero(client[curClientNo].nickname, 21);

    dup2(stdoutfd, STDOUT_FILENO);
    printf("client #%d left.\n", curClientNo+1);fflush(stdout);
    return;
}

int sem_create(key_t key, int initval) {
    register int id, semval;

    if(key == IPC_PRIVATE) return -1;
    else if(key == (key_t)-1) return -1;

    if( (id = semget(key, 3, 0666|IPC_CREAT)) < 0 ) return -1;

    /*** sem_num:2 value=value ***/
    if(semop(id, &op_lock[0], 2) < 0) write(STDERR_FILENO, "cannot lock\n", 12);

    if( (semval = semctl(id, 1, GETVAL, 0)) < 0) write(STDERR_FILENO, "cannot GETVAL\n", 14);

    if(semval == 0) {

        /*** sem_num:0  ***/
        semctl_arg.val = initval;
        if(semctl(id, 0, SETVAL, semctl_arg) < 0) write(STDERR_FILENO, "cannot SETVAL[0]\n", 19);

        /*** sem_num:1 ***/
        semctl_arg.val = BIGCOUNT;
        if(semctl(id, 1, SETVAL, semctl_arg) < 0) write(STDERR_FILENO, "cannot SETVAL[1]\n", 19);
    }

    /*** sem_num:1 value=value-1 ***/
    if(semop(id, &op_endcreate[0], 2) < 0) write(STDERR_FILENO, "cannot end create\n", 18);

    return id;
}

int sem_open(key_t key) {
    register int id;
    if(key == IPC_PRIVATE) return -1;
    else if(key == (key_t)-1) return -1;

    if( (id = semget(key, 3, 0)) < 0) return -1;

    /*** sem_num:1 ***/
    if(semop(id, &op_open[0], 1) < 0) write(STDERR_FILENO, "cannot open\n", 12);

    return id;
}

void sem_rm(int id) {
    if(semctl(id, 0, IPC_RMID, 0) < 0) write(STDERR_FILENO, "cannot IPC_RMID\n", 16);
    return;
}

void sem_close(int id) {
    register int semval;

    /*** sem_num:2 value=value ***/
    if(semop(id, &op_close[0], 3) < 0) write(STDERR_FILENO, "cannot semop\n", 13);

    if( (semval = semctl(id, 1, GETVAL, 0)) < 0) write(STDERR_FILENO, "cannot GETVAL\n", 14);

    if(semval > BIGCOUNT) write(STDERR_FILENO, "sem[1] > BIGCOUNT\n", 18);
    else if(semval == BIGCOUNT) sem_rm(id);
    else if(semop(id, &op_unlock[0], 1) < 0) write(STDERR_FILENO, "cannot unlock\n", 14);
    /*** sem_num:2 value=value-1 ***/

    return;
}

void sem_op(int id, int value) {

   /*** sem_num:0 ***/ 
   if( (op_op[0].sem_op = value) == 0)  write(STDERR_FILENO, "cannot have value == 0\n", 23);
   if(semop(id, &op_op[0], 1) < 0)  write(STDERR_FILENO, "sem_op error\n", 13);
   return;
}

void sem_wait(int id) {
    sem_op(id, -1);
    return;
}

void sem_signal(int id) {
    sem_op(id, 1);
    return;
}
