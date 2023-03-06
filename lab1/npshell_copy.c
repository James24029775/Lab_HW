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
#define true 1
#define false 0

void split_terms(char *, char *[LENGTH_LIMIT][5], int *, int *, char *);
void show(char *[LENGTH_LIMIT][5], int, int *);
void printenv(char **, int);
// void ls(char **, int);
// void cat(char **, int);
// void removetag(char **, int);
// void removetag0(char **, int);
// void number(char **, int);
void bin_func(char *[LENGTH_LIMIT][5], int, int *, char *);
int check_valid(char *[LENGTH_LIMIT][5], int, char *);

int main(void) {
    char input[LENGTH_LIMIT], illegal_term[LENGTH_LIMIT], filename[LENGTH_LIMIT];
    ;
    int cmd_amt, terms_amt[LENGTH_LIMIT], cmd_cnt;

    printf("%% ");
    while (fgets(input, LENGTH_LIMIT, stdin)) {
        // reset
        cmd_amt = 0;
        memset(terms_amt, 0, LENGTH_LIMIT);
        memset(filename, 0, LENGTH_LIMIT);
        char *terms[LENGTH_LIMIT][5];
        // ......................

        split_terms(input, terms, &cmd_amt, terms_amt, filename);
        // show(terms, cmd_amt, terms_amt);
        if (!check_valid(terms, cmd_amt, illegal_term)) {
            printf("%s\n", illegal_term);
            printf("\n%% ");
            continue;
        }
        //! I think that i don't need while here, because only bin_func need to tackle pipe.

        //! Maybe i should combine ls, cat, removetag, etc into a function.
        if (!strcmp(terms[0][0], "printenv")) {
            printenv(terms[0], terms_amt[0]);
        } else if (!strcmp(terms[0][0], "setenv")) {
            setenv(terms[0][1], terms[0][2], 1);
        } else if (!strcmp(terms[0][0], "exit")) {
            exit(EXIT_SUCCESS);
        } else {
            bin_func(terms, cmd_amt, terms_amt, filename);
        }

        // else if (!strcmp(*terms, "ls")) {
        //     ls(terms, cmd_amt);
        // } else if (!terms[cmd_cnt][0], "cat") {
        //     // bin_func(terms[cmd_cnt], terms_amt[cmd_cnt]);
        //     cat(terms[cmd_cnt], terms_amt[cmd_cnt]);

        // } else if (!strcmp(*terms, "removetag")) {
        //     removetag(terms, cmd_amt);
        // } else if (!strcmp(*terms, "removetag0")) {
        //     removetag0(terms, cmd_amt);
        // } else if (!strcmp(*terms, "number")) {
        //     number(terms, cmd_amt);

        printf("\n%% ");
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

    i = 0;
    memset(d, 0, 10);
    strcpy(d, "  ");
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
    *cmd_amt = i;
}

void show(char *terms[LENGTH_LIMIT][5], int cmd_amt, int *terms_amt) {
    int x, y;
    for (x = 0; x < cmd_amt; x++) {
        for (y = 0; y < terms_amt[x]; y++) {
            printf("%s,", terms[x][y]);
        }
        printf("\n");
    }
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

// void ls(char **terms, int cmd_amt) {
//     pid_t pid;
//     pid = fork();
//     switch (pid) {
//         case -1:
//             perror("Error");
//             exit(EXIT_FAILURE);
//             break;
//         case 0:
//             if (cmd_amt == 1) {
//                 execlp("ls", "ls", NULL);
//             } else {
//                 execlp("ls", "ls", *(terms + 1), NULL);
//             }
//             perror("Error");
//             exit(EXIT_FAILURE);
//             break;
//         default:
//             // Not sure should i use waitpid?
//             // https://wirelessr.gitbooks.io/working-life/content/wait_vs_waitpid.html
//             wait(NULL);
//             break;
//     }
// }

// void cat(char **terms, int cmd_amt) {
//     pid_t pid;
//     pid = fork();
//     switch (pid) {
//         case -1:
//             perror("Error");
//             exit(EXIT_FAILURE);
//             break;
//         case 0:
//             if (cmd_amt == 1) {
//                 execlp("cat", "cat", NULL);
//             } else {
//                 execlp("cat", "cat", *(terms + 1), NULL);
//             }
//             perror("Error");
//             exit(EXIT_FAILURE);
//             break;
//         default:
//             // Not sure should i use waitpid?
//             // https://wirelessr.gitbooks.io/working-life/content/wait_vs_waitpid.html
//             wait(NULL);
//             break;
//     }
// }

// void removetag(char **terms, int cmd_amt) {
//     pid_t pid;
//     pid = fork();
//     switch (pid) {
//         case -1:
//             perror("Error");
//             exit(EXIT_FAILURE);
//             break;
//         case 0:
//             //! It may be wrong, but tackle it later.
//             if (cmd_amt == 1) {
//                 execlp("removetag", "removetag", NULL);
//             }
//             //! bin/ hasn't added into env PATH, but tackle it later.
//             else {
//                 execlp("bin/removetag", "removetag", *(terms + 1), NULL);
//             }
//             perror("Error");
//             exit(EXIT_FAILURE);
//             break;
//         default:
//             // Not sure should i use waitpid?
//             // https://wirelessr.gitbooks.io/working-life/content/wait_vs_waitpid.html
//             wait(NULL);
//             break;
//     }
// }

// void removetag0(char **terms, int cmd_amt) {
//     pid_t pid;
//     pid = fork();
//     switch (pid) {
//         case -1:
//             perror("Error");
//             exit(EXIT_FAILURE);
//             break;
//         case 0:
//             //! It may be wrong, but tackle it later.
//             if (cmd_amt == 1) {
//                 execlp("removetag0", "removetag0", NULL);
//             }
//             //! bin/ hasn't added into env PATH, but tackle it later.
//             else {
//                 execlp("bin/removetag0", "removetag0", *(terms + 1), NULL);
//             }
//             perror("Error");
//             exit(EXIT_FAILURE);
//             break;
//         default:
//             // Not sure should i use waitpid?
//             // https://wirelessr.gitbooks.io/working-life/content/wait_vs_waitpid.html
//             wait(NULL);
//             break;
//     }
// }

// void number(char **terms, int cmd_amt) {
//     pid_t pid;
//     pid = fork();
//     switch (pid) {
//         case -1:
//             perror("Error");
//             exit(EXIT_FAILURE);
//             break;
//         case 0:
//             //! It may be wrong, but tackle it later.
//             if (cmd_amt == 1) {
//                 execlp("number", "number", NULL);
//             }
//             //! bin/ hasn't added into env PATH, but tackle it later.
//             else {
//                 execlp("bin/number", "number", *(terms + 1), NULL);
//             }
//             perror("Error");
//             exit(EXIT_FAILURE);
//             break;
//         default:
//             // Not sure should i use waitpid?
//             // https://wirelessr.gitbooks.io/working-life/content/wait_vs_waitpid.html
//             wait(NULL);
//             break;
//     }
// }

// "ls", "cat", "removetag", "removetag0", "number"
void bin_func(char *terms[LENGTH_LIMIT][5], int cmd_amt, int *terms_amt, char *filename) {
    int cmd_cnt, fd;
    pid_t pid;
    pid = fork();

    cmd_cnt = 0;
    // for (cmd_cnt=0; cmd_cnt < cmd_amt ;cmd_cnt++)

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
            //! It may be wrong, but tackle it later.
            if (terms_amt[cmd_cnt] == 1) {
                execlp(terms[cmd_cnt][0], terms[cmd_cnt][0], NULL);
            } else if (terms_amt[cmd_cnt] > 1) {
                // show(terms, cmd_amt, terms_amt);
                execlp(terms[cmd_cnt][0], terms[cmd_cnt][0], terms[cmd_cnt][1], NULL);
            }
            perror("Error");
            exit(EXIT_FAILURE);
        default:
            // Not sure should i use waitpid?
            // https://wirelessr.gitbooks.io/working-life/content/wait_vs_waitpid.html
            wait(NULL);
            break;
    }
}

int check_valid(char *terms[LENGTH_LIMIT][5], int cmd_amt, char *illegal_term) {
    int i, j, legal_flg;
    char *valid_cmd[VALID_CMD_NUM] = VALID_CMD;

    for (i = 0; i < cmd_amt; i++) {
        legal_flg = false;
        for (j = 0; j < VALID_CMD_NUM; j++) {
            if (!strcmp(terms[i][0], valid_cmd[j])) {
                legal_flg = true;
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