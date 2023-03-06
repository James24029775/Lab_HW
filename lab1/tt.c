#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define LENGTH_LIMIT 15000
#define NP_AMT_LIMIT 10
#define CMD_AMT_LIMIT 1000
#define TERM_AMT_LIMIT 5
#define CHAR_AMT_LIMIT 100
#define NP_LEN_LIMIT 1000
#define VALID_CMD "printenv", "setenv", "ls", "cat", "removetag", "removetag0", "number"
#define bool int
#define true 1
#define false 0

struct term {
    char string[CHAR_AMT_LIMIT];
};

struct cmd {
    struct term term[TERM_AMT_LIMIT];
    int term_cnt;
};

struct np {
    struct cmd cmd[CMD_AMT_LIMIT];
    int cmd_cnt;
    int ttl;
};

struct parallel {
    struct np np[NP_AMT_LIMIT];
    int np_cnt;
};

struct zeroIndexData {
    int parallel_np[NP_AMT_LIMIT][NP_LEN_LIMIT];
    int parallel_cnt;
    int np_cnt[NP_AMT_LIMIT];
};

int tran2number(char *term) {
    char *p = term;
    int num = 0;
    while (*p) {
        num *= 10;
        num += (*p - '0');
        p++;
    }
    return num;
}

void minus(struct parallel *parallel_np) {
    int np_cnt, i;
    struct np *local_np;

    np_cnt = parallel_np->np_cnt;
    for (i = 0; i < np_cnt; i++) {
        local_np = &(parallel_np->np[i]);
        local_np->ttl--;
    }
}

void show(struct parallel *parallel_np) {
    int np_cnt = parallel_np->np_cnt;
    struct np *local_np;
    int cmd_cnt, term_cnt;
    struct cmd *local_cmd;
    struct term *local_term;

    int i, j, k;
    printf("*************************\n");
    printf("Show np:\n");
    for (i = 0; i < np_cnt; i++) {
        local_np = &(parallel_np->np[i]);
        cmd_cnt = local_np->cmd_cnt;
        printf("%d\n", local_np->ttl);
        for (j = 0; j < cmd_cnt; j++) {
            local_cmd = &(local_np->cmd[j]);
            term_cnt = local_cmd->term_cnt;
            for (k = 0; k < term_cnt; k++) {
                local_term = &(local_cmd->term[k]);
                printf("%s\t", local_term->string);
            }
            printf("\n");
        }
    }
}

int storeCmd(struct parallel *parallel_np, char *cmd[100][100], int terms_amt[], int cmd_amt) {
    //! I have to compare cmd_amt[1][0] to np's ttl, if same then queue to same np struct
    int *np_cnt;
    struct np *local_np;
    int *cmd_cnt;
    struct cmd *local_cmd;
    int *term_cnt;
    struct term *local_term;
    int i, np_index;
    bool diffGen_flg;

    np_cnt = &(parallel_np->np_cnt);
    local_np = &(parallel_np->np[*np_cnt]);
    diffGen_flg = true;
    for (i = 0; i < *np_cnt; i++) {
        local_np = &(parallel_np->np[i]);
        if (local_np->ttl == tran2number(cmd[1][0])) {
            np_index = i;
            diffGen_flg = false;
            break;
        }
    }
    if (diffGen_flg) {
        // Because generally index minus 1 equals to *np_cnt, the order should be like bellow.
        np_index = *np_cnt;
        (*np_cnt)++;
    }

    local_np = &(parallel_np->np[np_index]);
    cmd_cnt = &(local_np->cmd_cnt);
    local_cmd = &(local_np->cmd[*cmd_cnt]);
    term_cnt = &(local_cmd->term_cnt);
    local_term = &(local_cmd->term[*term_cnt]);

    for (i = 0; i < terms_amt[0]; i++) {
        strcpy(local_term->string, cmd[0][i]);
        (*term_cnt)++;
        local_term = &(local_cmd->term[*term_cnt]);
    }

    (*cmd_cnt)++;
    local_np->ttl = tran2number(cmd[1][0]);
    return tran2number(cmd[1][0]);
}

void numbered_pipe(struct parallel *parallel_np, struct zeroIndexData *zeroIndex, char *cmd[100][100]) {
    int *np_cnt, *cmd_cnt, *term_cnt, fds[2];
    pid_t pid;
    struct cmd *local_cmd;
    struct term *local_term;
    struct np *local_np;
    int target_index;


    if (pipe(fds) == -1) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    //! should i use recursion? 
    // pid = fork();
    // switch (pid) {
    //     case -1:
    //         perror("Error");
    //         exit(EXIT_FAILURE);
    //     case 0:
    //         dup2(fds[1], STDOUT_FILENO);
    //         close(fds[0]);
    //         close(fds[1]);
    //         ordinary_pipe(terms, index - 1);
    //         perror("Error");
    //         exit(EXIT_FAILURE);
    //     default:
    //         dup2(fds[0], STDIN_FILENO);
    //         close(fds[0]);
    //         close(fds[1]);
    //         execlp(cmd[index][0], cmd[index][0], NULL);
    //         perror("Error");
    //         exit(EXIT_FAILURE);
    // }

    // cmd_cnt = &(np->cmd_cnt);
    // local_cmd = &(np->cmd[*cmd_cnt]);
}

void update_zeroIndex(struct zeroIndexData *zeroIndex, int index0, int index1) {
    int *parallel_cnt = &(zeroIndex->parallel_cnt);
    int parallel_index;
    int *np_index;
    bool foundflg;

    if (index0 == -1 || index1 == -1) {
        return;
    }

    foundflg = false;
    int i, j, target_index;
    for (i = 0; i < *parallel_cnt; i++) {
        for (j = 0; j < zeroIndex->np_cnt[i]; j++) {
            if (zeroIndex->parallel_np[i][j] == index0) {
                target_index = i;
                foundflg = true;
            }
        }
    }
    if (foundflg) {
        np_index = &(zeroIndex->np_cnt[target_index]);
        zeroIndex->parallel_np[target_index][*np_index] = index1;
        (*np_index)++;
    } else {
        np_index = &(zeroIndex->np_cnt[*parallel_cnt]);
        zeroIndex->parallel_np[*parallel_cnt][*np_index] = index0;
        (*np_index)++;
        zeroIndex->parallel_np[*parallel_cnt][*np_index] = index1;
        (*np_index)++;
        (*parallel_cnt)++;
    }
}

void show_zeroIndex(struct zeroIndexData *zeroIndex) {
    int i, j;
    int parallel_cnt = zeroIndex->parallel_cnt;
    int np_cnt;
    printf("*************************\n");
    printf("Show zeroIndex:\n");
    for (i = 0; i < parallel_cnt; i++) {
        np_cnt = zeroIndex->np_cnt[i];
        for (j = 0; j < np_cnt; j++) {
            printf("%d ", zeroIndex->parallel_np[i][j]);
        }
        printf("\n");
    }
}

void initial_zeroIndex(struct zeroIndexData *zeroIndex) {
    zeroIndex->parallel_cnt = 0;
}

int getTTLIndex(struct parallel *parallel_np, int target_ttl) {
    int np_cnt, i;
    struct np *local_np;
    struct cmd *local_cmd;
    struct term *local_term;

    np_cnt = parallel_np->np_cnt;
    for (i = 0; i < np_cnt; i++) {
        local_np = &(parallel_np->np[i]);
        if (local_np->ttl == target_ttl) {
            return i;
        }
    }
    return -1;
}

void bin_func(char *terms[100][100], int cmd_amt, int *terms_amt, char *filename, bool ifNumberPipe, struct parallel *parallel_np, struct zeroIndexData *zeroIndex) {
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
            } else if (ifNumberPipe) {
                numbered_pipe(parallel_np, zeroIndex, terms);
            } else {
                // ordinary_pipe(terms, cmd_amt - 1);
            }

        default:
            // Not sure should i use waitpid?
            // https://wirelessr.gitbooks.io/working-life/content/wait_vs_waitpid.html
            wait(NULL);
            break;
    }
}
// removetag test.html |2
// 2 removetag test.html
// 1 2
int main(void) {
    char *cmd1[100][100] = {{"removetag", "test.html"}, {"2"}};
    char *cmd2[100][100] = {{"numbererer", "test.html"}, {"2"}};
    char *cmd3[100][100] = {{"number"}, {"2"}};
    char *cmd4[100][100] = {{"number"}, {"2"}};
    char *cmd5[100][100] = {{"number"}, {"2"}};
    char *cmd6[100][100] = {{"number"}};

    int terms_amt1[] = {2, 1};
    int terms_amt2[] = {2, 1};
    int terms_amt3[] = {1, 1};
    int terms_amt4[] = {1, 1};
    int terms_amt5[] = {1, 1};
    int terms_amt6[] = {1};
    int index, index0, index1;

    int cmd_amt1 = 2;
    int cmd_amt2 = 2;
    int cmd_amt3 = 2;
    int cmd_amt4 = 2;
    int cmd_amt5 = 2;
    int cmd_amt6 = 1;
    int ttl;

    struct parallel parallel_np;
    struct zeroIndexData zeroIndex;
    memset(&parallel_np, 0, sizeof(parallel_np));
    memset(&zeroIndex, 0, sizeof(zeroIndex));
    initial_zeroIndex(&zeroIndex);

    bool ifNumberPipe;
    char *filename;
    struct np *np_ptr;

    ifNumberPipe = true;
    minus(&parallel_np);
    if (ifNumberPipe) {
        ttl = storeCmd(&parallel_np, cmd1, terms_amt1, cmd_amt1);
        index0 = getTTLIndex(&parallel_np, 0);
        index1 = getTTLIndex(&parallel_np, ttl);
        update_zeroIndex(&zeroIndex, index0, index1);
    } else if ((index = getTTLIndex(&parallel_np, 0)) != -1) {
        bin_func(cmd1, cmd_amt1, terms_amt1, filename, ifNumberPipe, &parallel_np, &zeroIndex);
    } else {
    }
    show(&parallel_np);
    show_zeroIndex(&zeroIndex);

    ifNumberPipe = true;
    minus(&parallel_np);
    if (ifNumberPipe) {
        ttl = storeCmd(&parallel_np, cmd1, terms_amt1, cmd_amt1);
        index0 = getTTLIndex(&parallel_np, 0);
        index1 = getTTLIndex(&parallel_np, ttl);
        update_zeroIndex(&zeroIndex, index0, index1);
    } else if ((index = getTTLIndex(&parallel_np, 0)) != -1) {
        np_ptr = &(parallel_np.np[index]);
        bin_func(cmd1, cmd_amt1, terms_amt1, filename, ifNumberPipe, &parallel_np, &zeroIndex);
    } else {
    }
    show(&parallel_np);
    show_zeroIndex(&zeroIndex);

    ifNumberPipe = false;
    minus(&parallel_np);
    if (ifNumberPipe) {
        ttl = storeCmd(&parallel_np, cmd6, terms_amt6, cmd_amt6);
        index0 = getTTLIndex(&parallel_np, 0);
        index1 = getTTLIndex(&parallel_np, ttl);
        update_zeroIndex(&zeroIndex, index0, index1);
    } else if ((index = getTTLIndex(&parallel_np, 0)) != -1) {
        np_ptr = &(parallel_np.np[index]);
        bin_func(cmd6, cmd_amt6, terms_amt6, filename, ifNumberPipe, &parallel_np, &zeroIndex);
    } else {
    }
    show(&parallel_np);
    show_zeroIndex(&zeroIndex);

    // ifNumberPipe = true;
    // minus(&parallel_np);
    // if (ifNumberPipe) {
    //     ttl = storeCmd(&parallel_np, cmd1, terms_amt1, cmd_amt1);
    //     index0 = getTTLIndex(&parallel_np, 0);
    //     index1 = getTTLIndex(&parallel_np, ttl);
    //     update_zeroIndex(&zeroIndex, index0, index1);
    // } else if ((index = getTTLIndex(&parallel_np, 0)) != -1) {
    //     np_ptr = &(parallel_np.np[index]);
    //     bin_func(cmd1, cmd_amt1, terms_amt1, filename, ifNumberPipe, np_ptr, &zeroIndex);
    // } else {
    // }
    // show(&parallel_np);
    // show_zeroIndex(&zeroIndex);

    // ifNumberPipe = true;
    // minus(&parallel_np);
    // if (ifNumberPipe) {
    //     ttl = storeCmd(&parallel_np, cmd1, terms_amt1, cmd_amt1);
    //     index0 = getTTLIndex(&parallel_np, 0);
    //     index1 = getTTLIndex(&parallel_np, ttl);
    //     update_zeroIndex(&zeroIndex, index0, index1);
    // } else if ((index = getTTLIndex(&parallel_np, 0)) != -1) {
    //     np_ptr = &(parallel_np.np[index]);
    //     bin_func(cmd1, cmd_amt1, terms_amt1, filename, ifNumberPipe, np_ptr, &zeroIndex);
    // } else {
    // }
    // show(&parallel_np);
    // show_zeroIndex(&zeroIndex);

    // ifNumberPipe = true;
    // minus(&parallel_np);
    // if (ifNumberPipe) {
    //     ttl = storeCmd(&parallel_np, cmd1, terms_amt1, cmd_amt1);
    //     index0 = getTTLIndex(&parallel_np, 0);
    //     index1 = getTTLIndex(&parallel_np, ttl);
    //     update_zeroIndex(&zeroIndex, index0, index1);
    // } else if ((index = getTTLIndex(&parallel_np, 0)) != -1) {
    //     np_ptr = &(parallel_np.np[index]);
    //     bin_func(cmd1, cmd_amt1, terms_amt1, filename, ifNumberPipe, np_ptr, &zeroIndex);
    // } else {
    // }
    // show(&parallel_np);
    // show_zeroIndex(&zeroIndex);
}