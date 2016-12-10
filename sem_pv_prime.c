#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h> 
#include <sys/shm.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define START 10010001
#define END 10020000
#define NPROC 4

static int pvid;

int mysem_init(int n)
{
    int semid;

    semid = semget(IPC_PRIVATE, 1, IPC_CREAT|0600);
    if (semid < 0) {
        perror("semget()");
        return -1;
    }
    if (semctl(semid, 0, SETVAL, n) < 0) {
        perror("semctl()");
        return -1;
    }
    return semid;
}

void mysem_destroy(int pvid)
{
    semctl(pvid, 0, IPC_RMID);
}

int P(int pvid)
{
    struct sembuf sbuf;

    sbuf.sem_num = 0;
    sbuf.sem_op = -1;
    sbuf.sem_flg = 0;

    while (semop(pvid, &sbuf, 1) < 0) {
        if (errno == EINTR) {
            continue;
        }
        perror("semop(p)");
        return -1;
    }

    return 0;
}

int V(int pvid)
{
    struct sembuf sbuf;

    sbuf.sem_num = 0;
    sbuf.sem_op = 1;
    sbuf.sem_flg = 0;

    if (semop(pvid, &sbuf, 1) < 0) {
        perror("semop(v)");
        return -1;
    }
    return 0;
}

int prime_proc(int n)
{
    int i, j, flag;

    flag = 1;
    for (i=2;i<n/2;++i) {
        if (n%i == 0) {
            flag = 0;
            break;
        }
    }
    if (flag == 1) {
        printf("%d is a prime\n", n);
    }
    V(pvid);
    exit(0);
}

void sig_child(int sig_num)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(void)
{
    pid_t pid;
    int i;

    if (signal(SIGCHLD, sig_child) == SIG_ERR) {
        perror("signal()");
        exit(1);
    }

    pvid = mysem_init(NPROC);

    for (i = START; i < END; i += 2) {
        P(pvid);
        pid = fork();
        if (pid < 0) {
            V(pvid);
            perror("fork()");
            exit(1);
        }
        if (pid == 0) {
            prime_proc(i);
        }
    }
    while (1) {sleep(1);};
    mysem_destroy(pvid);
    exit(0);
}
