#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>

int main()
{
    int fds[2]={0};

    if (pipe(fds) == -1){
        perror("Pipe Error.\n");
        exit(EXIT_FAILURE);
    }
    pid_t pid;
    pid = fork();
    if (pid == -1){
        perror("Fork Error.\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0){
        dup2(fds[1], STDOUT_FILENO);
        close(fds[0]);
        close(fds[1]);
        // 為何需要打兩次
        execlp("ls", "ls", NULL);
        fprintf(stderr, "ls error.\n");
        exit(EXIT_FAILURE);
    }
    dup2(fds[0], STDIN_FILENO);
    close(fds[0]);
    close(fds[1]);
    execlp("wc", "wc", "-w", NULL);
    fprintf(stderr, "wc error.\n");
    exit(EXIT_FAILURE);
}