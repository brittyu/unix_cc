#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define STRING "hello world!"

int
main()
{
    int pipefd[2];
    pid_t pid;
    char buf[BUFSIZ];

    if (pipe(pipefd) == -1) {
        perror("pipe()");
        exit(1);
    } 
    pid = fork();
    if (pid == -1) {
        perror("fork()");
        exit(1);
    }

    if (pid == 0) {
        printf("child pid is: %d\n", getpid());
        if (read(pipefd[0], buf, BUFSIZ) < 0) {
            perror("write()");
            exit(1);
        }
        
        printf("%s\n", buf);
        bzero(buf, BUFSIZ);

        snprintf(buf, BUFSIZ, "Message from child: My pid is: %d", getpid());

        if (write(pipefd[1], buf, strlen(buf)) < 0) {
            perror("write()");
            exit(1);
        }
    } else {
        printf("Parent pid is: %d\n", getpid());

        snprintf(buf, BUFSIZ, "Message from parent: My pid is: %d", getpid());

        if (write(pipefd[1], buf, strlen(buf)) < 0) {
            perror("write()");
            exit(1);
        }

        sleep(1);

        bzero(buf, BUFSIZ);
        if (read(pipefd[0], buf, BUFSIZ) < 0) {
            perror("write()");
            exit(1);
        }

        printf("%s\n", buf);
        wait(NULL);
    }

    exit(0);
}
