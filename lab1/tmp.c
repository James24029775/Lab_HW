#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define LENGTH_LIMIT 15000
#define VALID_CMD "printenv", "setenv", "ls", "cat", "removetag", "removetag0", "number"
#define true 1
#define false 0

void ordinary_pipe(char *terms[100][100], int index) {
    if (index == 0) {
        execlp(terms[index][0], terms[index][0], terms[index][1], NULL);
        return;
    }

    int fds[2];
    if (pipe(fds) == -1) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    pid_t pid;
    pid = fork();

    switch (pid) {
        case -1:
            perror("Error");
            exit(EXIT_FAILURE);
        case 0:
            dup2(fds[1], STDOUT_FILENO);
            close(fds[0]);
            close(fds[1]);
            ordinary_pipe(terms, index - 1);
            perror("Error");
            exit(EXIT_FAILURE);
        default:
            dup2(fds[0], STDIN_FILENO);
            close(fds[0]);
            close(fds[1]);
            execlp(terms[index][0], terms[index][0], NULL);
            perror("Error");
            exit(EXIT_FAILURE);
    }
}

int isdigit_myself(char *term) {
    int i, flg;
    flg = true;
    printf("TTTTTTTTTTTTTTT\n");
    for (i = 0; i < strlen(term); i++) {
        if (!isdigit(term[i])) {
            flg = false;
        }
    }
    return flg;
}

// bin/number test.html | bin/removetag
// bin/number test.html | bin/removetag | bin/number
// bin/removetag test.html | bin/number | bin/number | bin/number
// ls bin | cat
// ls bin | cat | cat | cat | cat
int main(void) {
    // char *tmp[100][100] = {{"ls", "bin"}, {"cat"}, {"bin/number"}};
    // char *tmp[100][100] = {{"bin/number", "test.html"}, {"bin/number"}};
    // char *tmp[100][100] = {{"bin/removetag", "test.html"}, {"bin/number"}, {"bin/number"}, {"bin/number"}};
    // char *tmp[100][100] = {{"ls", "bin"}, {"cat"}, {"cat"}, {"cat"}, {"cat"}};
    // int amt[100] = {2, 1, 1}, cmd_amt = 5;
    // pid_t pid;
    // pid = fork();

    // switch (pid) {
    //     case -1:
    //         perror("Error");
    //         exit(EXIT_FAILURE);
    //     case 0:
    //         ordinary_pipe(tmp, cmd_amt - 1);

    //     default:
    //         // Not sure should i use waitpid?
    //         // https://wirelessr.gitbooks.io/working-life/content/wait_vs_waitpid.html
    //         wait(NULL);
    //         break;
    // }
    char *tmp = "9876544321";
    tmp = "FFFFFF";
    // printf("%p\n", "ffffffff");
    // printf("%c\n", *("ffffffff"));
    // printf("%p\n", "ffffffff"+1);
    // printf("%c\n", *("ffffffff"+1));
    // printf("%p\n", "ffffffff"+2);
    // printf("%c\n", *("ffffffff"+2));
    // printf("%p\n", "ffffffff"+3);
    // printf("%c\n", *("ffffffff"+3));
    printf("%p\n", (tmp));
    printf("%c\n", *(tmp));
    printf("%p\n", (tmp + 1));
    printf("%c\n", *(tmp + 1));
    printf("%p\n", (tmp + 2));
    printf("%c\n", *(tmp + 2));

    // char *tmp[100][100] = {{"ls", "bin"}, {"cat"}, {"bin/number"}};
    // printf("%s\n", tmp[0][0]);
    // tmp[0][0] = "FEFEFEF";
    // printf("%s\n", tmp[0][0]);
}