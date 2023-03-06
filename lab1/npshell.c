#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define LENGTH_LIMIT 15000
#define VALID_CMD \
    { "printenv", "setenv", "ls", "cat", "removetag", "removetag0", "number", "exit" }
#define VALID_CMD_NUM 8
#define bool int
#define true 1
#define false 0

//! Do Numbered pipe!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void split_terms(char *, char *[LENGTH_LIMIT][5], int *, int *, char *);
void show(char *[LENGTH_LIMIT][5], int, int *);
void printenv(char **, int);
void bin_func(char *[LENGTH_LIMIT][5], int, int *, char *);
void trim(char *);
void ordinary_pipe(char *[LENGTH_LIMIT][5], int);
bool check_valid(char *[LENGTH_LIMIT][5], int, char *, bool *);
bool check_numbered_pipe(char *[LENGTH_LIMIT][5], int);
bool isdigit_myself(char *);

int main(void) {
    char input[LENGTH_LIMIT], illegal_term[LENGTH_LIMIT], filename[LENGTH_LIMIT];
    int cmd_amt, terms_amt[LENGTH_LIMIT];
    bool ifNumberPipe;

    printf("%% ");
    while (fgets(input, LENGTH_LIMIT, stdin)) {
        // reset
        char *terms[LENGTH_LIMIT][5];
        memset(terms_amt, 0, LENGTH_LIMIT);
        memset(filename, 0, LENGTH_LIMIT);
        cmd_amt = 0;
        ifNumberPipe = false;
        split_terms(input, terms, &cmd_amt, terms_amt, filename);
        memset(input, 0, LENGTH_LIMIT);
        // ......................

        if (!check_valid(terms, cmd_amt, illegal_term, &ifNumberPipe)) {
            printf("%s\n", illegal_term);
            printf("%% ");
            continue;
        }

        show(terms, cmd_amt, terms_amt);
        if (ifNumberPipe) {
            printf("SHIT");
        } else {
            if (!strcmp(terms[0][0], "printenv")) {
                printenv(terms[0], terms_amt[0]);
            } else if (!strcmp(terms[0][0], "setenv")) {
                setenv(terms[0][1], terms[0][2], 1);
            } else if (!strcmp(terms[0][0], "exit")) {
                exit(EXIT_SUCCESS);
            } else {
                bin_func(terms, cmd_amt, terms_amt, filename);
            }
        }

        printf("%% ");
    }
}

void split_terms(char *input, char *terms[LENGTH_LIMIT][5], int *cmd_amt, int *terms_amt, char *filename) {
    char d[10];
    char *p, *commands;
    char *command[LENGTH_LIMIT];
    char local_input[LENGTH_LIMIT];
    int i, j;

    i = 0;
    memset(d, 0, 10);
    strcpy(d, ">");
    strcpy(local_input, input);
    local_input[strlen(local_input) - 1] = 0;
    p = strtok(local_input, d);
    while (p != NULL) {
        if (i++ == 0) {
            commands = p;
        } else {
            strcpy(filename, p);
            trim(filename);
        }
        // try strcpy, and see term's address if is same as p.
        p = strtok(NULL, d);
    }

    i = 0;
    memset(d, 0, 10);
    strcpy(d, "|");
    p = strtok(commands, d);
    while (p != NULL) {
        *(command + i) = p;
        i++;
        p = strtok(NULL, d);
    }
    *cmd_amt = i;

    i = 0;
    memset(d, 0, 10);
    strcpy(d, "  ");

    //! I just can't figure out why the last time array value would be kept.!!!!!!!!!!!!!!!!!!!!!!!
    char **t = command;
    while (*t) {
        j = 0;
        p = strtok(*t, d);
        while (p != NULL) {
            // I don't surely know why replace assign for strcpy would be wrong.
            terms[i][j++] = p;
            p = strtok(NULL, d);
            terms_amt[i]++;
        }
        i++;
        t++;
    }
    // char **nn = command;
    // while (*nn) {
    //     printf("%s\n", *nn);
    //     nn++;
    // }
}

void show(char *terms[LENGTH_LIMIT][5], int cmd_amt, int *terms_amt) {
    int x, y;
    printf("Commands amount: %d\n", cmd_amt);

    for (x = 0; x < cmd_amt; x++) {
        printf("%d\t", terms_amt[x]);
        for (y = 0; y < terms_amt[x]; y++) {
            printf("%s\t", terms[x][y]);
        }
        printf("\n");
    }
    printf("\n");
}

void printenv(char **terms, int cmd_amt) {
    if (cmd_amt == 1) {
        printf("Usage: printenv [var]");
    } else {
        char *s = getenv(*(terms + 1));
        if (s) {
            printf("%s", s);
        }
    }
}

// "ls", "cat", "removetag", "removetag0", "number"
void bin_func(char *terms[LENGTH_LIMIT][5], int cmd_amt, int *terms_amt, char *filename) {
    int fd;
    pid_t pid;
    pid = fork();

    switch (pid) {
        case -1:
            perror("Error");
            exit(EXIT_FAILURE);
        case 0:
            if (*filename) {
                fd = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
                if (fd == -1) {
                    perror("Error");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            if (cmd_amt == 1) {
                //! It may be wrong, but tackle it later. Only ls is correct.
                if (terms_amt[0] == 1) {
                    execlp(terms[0][0], terms[0][0], NULL);
                } else if (terms_amt[0] > 1) {
                    execlp(terms[0][0], terms[0][0], terms[0][1], NULL);
                }
                perror("Error");
                exit(EXIT_FAILURE);
            } else {
                ordinary_pipe(terms, cmd_amt - 1);
            }

        default:
            // Not sure should i use waitpid?
            // https://wirelessr.gitbooks.io/working-life/content/wait_vs_waitpid.html
            wait(NULL);
            break;
    }
}

void trim(char *str) {
    int cnt;
    char *p;
    cnt = 0;
    p = str;
    while (*p == ' ') {
        cnt++;
        p++;
    }
    memmove(str, p, strlen(str) - cnt + 1);
    p = str + strlen(str) - 1;
    while (*p == ' ') {
        *p = '\0';
        p--;
    }
}

void ordinary_pipe(char *terms[LENGTH_LIMIT][5], int index) {
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

bool check_valid(char *terms[LENGTH_LIMIT][5], int cmd_amt, char *illegal_term, bool *ifNumberPipe) {
    int i, j;
    bool legal_flg;
    char *valid_cmd[VALID_CMD_NUM] = VALID_CMD;

    for (i = 0; i < cmd_amt; i++) {
        legal_flg = false;
        for (j = 0; j < VALID_CMD_NUM; j++) {
            if (!strcmp(terms[i][0], valid_cmd[j])) {
                legal_flg = true;
                break;
            }
            if (isdigit_myself(terms[i][0])) {
                legal_flg = true;
                *ifNumberPipe = true;
                break;
            }
        }
        if (!legal_flg) {
            strcpy(illegal_term, terms[i][0]);
            return false;
        }
    }
    return true;
}

bool isdigit_myself(char *term) {
    int i;
    bool flg;
    flg = true;
    for (i = 0; i < strlen(term); i++) {
        if (!isdigit(term[i])) {
            flg = false;
        }
    }
    return flg;
}

bool check_numbered_pipe(char *terms[LENGTH_LIMIT][5], int cmd_amt) {
    int i, j;
    for (i = 0; i < cmd_amt; i++) {
        char p[LENGTH_LIMIT];
        strcpy(p, terms[i][0]);
    }
    return 0;
}