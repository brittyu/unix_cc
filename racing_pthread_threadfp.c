#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <wait.h>
#include <pthread.h>

#define COUNT 100
#define NUM 64
#define FILEPATH "/tmp/count"

void *do_child(void *p)
{
    int fd;
    int ret, count;
    char buf[NUM];
    FILE *filep;

    filep = fopen(FILEPATH, "r+");
    if (filep == NULL) {
        perror("fopen()");
        exit(1);
    }

    flockfile(filep);

    if (fseek(filep, 0L, SEEK_SET) == -1) {
        perror("fseek()");
    }
    ret = fread(buf, NUM, 1, filep);

    count = atoi(buf);
    ++count;
    sprintf(buf, "%d", count);
    if (fseek(filep, 0L, SEEK_SET) == -1) {
        perror("fseek()");
    }
    ret = fwrite(buf, strlen(buf), 1, filep);

    funlockfile(filep);

    fclose(filep);
    return NULL;
}

int main()
{
    pthread_t tid[COUNT];
    int count;


    for (count=0;count<COUNT;count++) {
        if (pthread_create(tid+count, NULL, do_child, NULL) != 0) {
            perror("pthread_create()");
            exit(1);
        }
    }

    for (count=0;count<COUNT;count++) {
        if (pthread_join(tid[count], NULL) != 0) {
            perror("pthread_join()");
            exit(1);
        }
    }


    exit(0);
}
