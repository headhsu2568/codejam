/******
 *
 * 'shared memory & semaphore functions'
 * by hEaDhSu
 *
 ******/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

/*** Define value ***/
#define SHMKEY ((key_t) 7890)
#define SEMKEY1 ((key_t) 6891)
#define SEMKEY2 ((key_t) 6892)
#define PERMS 0666
#define BIGCOUNT 99

#define SHM_SIZE 4096
#define MAX_MESGDATA_SIZE (4096-16)
#define MESG_HEADER_SIZE (sizeof(Mesg) - MAX_MESGDATA_SIZE)

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

/*** shm & sem variables ***/
int shmid, clisem, servsem;
struct Mesg* mesgptr;

/*** semaphore function declare ***/
int sem_create(key_t key, int initval);
int sem_open(key_t key);
void sem_rm(int id);
void sem_close(int id);
void sem_op(int id, int value);

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
