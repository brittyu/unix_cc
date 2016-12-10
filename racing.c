#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>
#include <wait.h>

#define COUNT 100
#define NUM 64
#define FILEPATH "/tmp/count"


int do_child(const char *path)
{
    int fd;
    int ret, count;
    char buf[NUM];
    fd = open(path, O_RDWR);
    ret = flock(fd, LOCK_EX);
    
    if (ret == -1) {
        perror("flock()");
        exit(1);
    }

    if (fd < 0) {
        perror("open()");
        exit(1);
    }

    ret = read(fd, buf, NUM);
    if (ret < 0) {
        perror("read()");
        exit(1);
    }

    buf[ret] = '\0';
    count = atoi(buf);
    ++count;

    sprintf(buf, "%d", count);
    lseek(fd, 0, SEEK_SET);
    ret = write(fd, buf, strlen(buf));

    ret = flock(fd, LOCK_UN);
    if (ret == -1) {
        perror("flock()");
        exit(1);
    }

    close(fd);
    exit(0);
}

int
main()
{
    pid_t pid;
    int count;

    for (count = 0; count < COUNT; count++) {
        pid = fork();
        if (pid < 0) {
            perror("fork()");
            exit(1);
        }

        if (pid == 0) {
            do_child(FILEPATH);
        }
    }

    for (count = 0; count < COUNT; count++) {
        wait(NULL);
    }
}
