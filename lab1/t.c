#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>


void ordinary_pipe(char t[100][100]) {
    int cmd_cnt = 4;
    int fds_size = (cmd_cnt-1)*2;

    int fds[cmd_cnt];
    int i, j;
    for (i = 0; i < cmd_cnt-1; i++){
        if (pipe(fds+i*2) < 0) {perror("pipe"); exit(1);};
    }

    pid_t pid;
    for ( i = 0; i < cmd_cnt; i++ ){
        pid = fork();
        if (pid == -1) {
            printf("fork failed\n");
            exit(1);
        }

        if (pid == 0){
            // printf("%s\n", t[i]);

            if (i != 0){
                if (dup2(fds[(i-1)*2], STDIN_FILENO) < 0) {perror("dup2"); exit(1);};
            }

            if (i != cmd_cnt-1){
                if (dup2(fds[2*i+1], STDOUT_FILENO) < 0) {perror("dup2"); exit(1);};
            }
            for ( j = 0; j < fds_size; j++ ){
                close(fds[j]);
            }

            execlp( t[i], t[i], NULL);
        }
    }
    for ( i = 0; i < fds_size; i++ ){
        close(fds[i]);
    }
 }
 void sig_handler(){
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0){}
    return;
}

int main()
{

    // char t[100][100] = {"ls", "bin/number", "bin/number", "wc"};
    // // char t1[100] = "ls";
    // // char t2[100] = "bin/number";
    // // char t3[100] = "wc";

    // ordinary_pipe(t);
    pid_t pid1, pid[100];
    // signal(SIGCHLD, SIG_IGN);
    int i;
    for(i = 0 ; i < 100 ;i ++){
        // pid[i] = fork();

        // if(pid[i] == 0){
        //     sleep(1);
        //     printf("Process %d.\n", getpid());
        //     exit(EXIT_SUCCESS);
        // }

        pid1 = fork();
        if (pid1 == 0){
            sleep(5);
            printf("child process sleep 5 sec\n");
            exit(EXIT_SUCCESS);
        }
    }
    sleep(10);
    printf("parent process sleep 10 sec\n");
    


    // printf("Process %d.\n", getpid());

}
