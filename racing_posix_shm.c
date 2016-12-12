#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

#define COUNT 100
#define SHMPATH "/shm"
#define SEMPATH "/sem"

static sem_t *sem;

sem_t *mylock_init(void)
{
    sem_t * ret;
    ret = sem_open(SEMPATH, O_CREAT|O_EXCL, 0600, 1);
    if (ret == SEM_FAILED) {
        perror("sem_open()");
        return NULL;
    }
    return ret;
}

void mylock_destroy(sem_t *sem)
{
    sem_close(sem);
    sem_unlink(SEMPATH);
}

int mylock(sem_t *sem)
{
    while (sem_wait(sem) < 0) {
        if (errno == EINTR) {
            continue;
        }
        perror("sem_wait()");
        return -1;
    }

    return 0;
}

int myunlock(sem_t *sem)
{
    if (sem_post(sem) < 0) {
        perror("semop()");
        return -1;
    }
}

int do_child(char * shmpath)
{
    int interval, shmfd, ret;
    int *shm_p;

    shmfd = shm_open(shmpath, O_RDWR, 0600);
    if (shmfd < 0) {
        perror("shm_open()");
        exit(1);
    }

    shm_p = (int *)mmap(NULL, sizeof(int), PROT_WRITE|PROT_READ, MAP_SHARED, shmfd, 0);
    if (MAP_FAILED == shm_p) {
        perror("mmap()");
        exit(1);
    }
    /* critical section */
    mylock(sem);
    interval = *shm_p;
    interval++;
    usleep(1);
    *shm_p = interval;
    myunlock(sem);
    /* critical section */
    munmap(shm_p, sizeof(int));
    close(shmfd);

    exit(0);
}

int main()
{
    pid_t pid;
    int count, shmfd, ret;
    int *shm_p;

    sem = mylock_init();
    if (sem == NULL) {
        fprintf(stderr, "mylock_init(): error!\n");
        exit(1);
    }

    shmfd = shm_open(SHMPATH, O_RDWR|O_CREAT|O_TRUNC, 0600);
    if (shmfd < 0) {
        perror("shm_open()");
        exit(1);
    }

    ret = ftruncate(shmfd, sizeof(int));
    if (ret < 0) {
        perror("ftruncate()");
        exit(1);
    }

    shm_p = (int *)mmap(NULL, sizeof(int), PROT_WRITE|PROT_READ, MAP_SHARED, shmfd, 0);
    if (MAP_FAILED == shm_p) {
        perror("mmap()");
        exit(1);
    }

    *shm_p = 0;

    for (count=0;count<COUNT;count++) {
        pid = fork();
        if (pid < 0) {
            perror("fork()");
            exit(1);
        }

        if (pid == 0) {
            do_child(SHMPATH);
        }
    }

    for (count=0;count<COUNT;count++) {
        wait(NULL);
    }

    printf("shm_p: %d\n", *shm_p);
    munmap(shm_p, sizeof(int));
    close(shmfd);
    shm_unlink(SHMPATH);
    sleep(3000);
    mylock_destroy(sem);
    exit(0);
}
