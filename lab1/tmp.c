#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define LENGTH_LIMIT 15000
#define CHAR_AMT_LIMIT 100
#define TERM_AMT_LIMIT 5
#define CMD_PER_NODE_LIMIT 100
#define CHILD_LIMIT 100
#define NODE_LIMIT 5000
#define VALID_CMD \
    "printenv", "setenv", "ls", "cat", "removetag", "removetag0", "number"
#define bool int
#define true 1
#define false 0

#define SIX                                                                       \
    {                                                                             \
        ifNumberPipe = false;                                                     \
        minus_ttl(&knowNode);                                                     \
        targetNode = getSpecificNode(&knowNode, 0);                               \
        if (ifNumberPipe) {                                                       \
            ttl = tran2number(cmd_6[1][0]);                                       \
            if (targetNode) {                                                     \
                parentNode = getSpecificNode(&knowNode, ttl);                     \
                if (parentNode) {                                                 \
                    addInfo2ExistNode(parentNode, cmd_6, terms_amt_6, cmd_amt_6); \
                    adoptChild(parentNode, targetNode);                           \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);    \
                    adoptChild(newNode, targetNode);                              \
                }                                                                 \
            } else {                                                              \
                targetNode = getSpecificNode(&knowNode, ttl);                     \
                if (targetNode) {                                                 \
                    addInfo2ExistNode(targetNode, cmd_6, terms_amt_6, cmd_amt_6); \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);    \
                }                                                                 \
            }                                                                     \
        } else if (targetNode) {                                                  \
            newNode = createNode(&knowNode);                                      \
            addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);            \
            adoptChild(newNode, targetNode);                                      \
            bin_func(cmd_6, cmd_amt_6, terms_amt_6, filename, ifNumberPipe,       \
                     newNode);                                                    \
        } else {                                                                  \
        }                                                                         \
    }
#define FIVE                                                                      \
    {                                                                             \
        ifNumberPipe = true;                                                      \
        minus_ttl(&knowNode);                                                     \
        targetNode = getSpecificNode(&knowNode, 0);                               \
        if (ifNumberPipe) {                                                       \
            ttl = tran2number(cmd_5[1][0]);                                       \
            if (targetNode) {                                                     \
                parentNode = getSpecificNode(&knowNode, ttl);                     \
                if (parentNode) {                                                 \
                    addInfo2ExistNode(parentNode, cmd_5, terms_amt_5, cmd_amt_5); \
                    adoptChild(parentNode, targetNode);                           \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_5, terms_amt_5, cmd_amt_5);    \
                    adoptChild(newNode, targetNode);                              \
                }                                                                 \
            } else {                                                              \
                targetNode = getSpecificNode(&knowNode, ttl);                     \
                if (targetNode) {                                                 \
                    addInfo2ExistNode(targetNode, cmd_5, terms_amt_5, cmd_amt_5); \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_5, terms_amt_5, cmd_amt_5);    \
                }                                                                 \
            }                                                                     \
        } else if (targetNode) {                                                  \
            newNode = createNode(&knowNode);                                      \
            addInfo2ExistNode(newNode, cmd_5, terms_amt_5, cmd_amt_5);            \
            adoptChild(newNode, targetNode);                                      \
            bin_func(cmd_5, cmd_amt_5, terms_amt_5, filename, ifNumberPipe,       \
                     newNode);                                                    \
        } else {                                                                  \
        }                                                                         \
    }
#define FOUR                                                                      \
    {                                                                             \
        ifNumberPipe = true;                                                      \
        minus_ttl(&knowNode);                                                     \
        targetNode = getSpecificNode(&knowNode, 0);                               \
        if (ifNumberPipe) {                                                       \
            ttl = tran2number(cmd_4[1][0]);                                       \
            if (targetNode) {                                                     \
                parentNode = getSpecificNode(&knowNode, ttl);                     \
                if (parentNode) {                                                 \
                    addInfo2ExistNode(parentNode, cmd_4, terms_amt_4, cmd_amt_4); \
                    adoptChild(parentNode, targetNode);                           \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_4, terms_amt_4, cmd_amt_4);    \
                    adoptChild(newNode, targetNode);                              \
                }                                                                 \
            } else {                                                              \
                targetNode = getSpecificNode(&knowNode, ttl);                     \
                if (targetNode) {                                                 \
                    addInfo2ExistNode(targetNode, cmd_4, terms_amt_4, cmd_amt_4); \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_4, terms_amt_4, cmd_amt_4);    \
                }                                                                 \
            }                                                                     \
        } else if (targetNode) {                                                  \
            newNode = createNode(&knowNode);                                      \
            addInfo2ExistNode(newNode, cmd_4, terms_amt_4, cmd_amt_4);            \
            adoptChild(newNode, targetNode);                                      \
            bin_func(cmd_4, cmd_amt_4, terms_amt_4, filename, ifNumberPipe,       \
                     newNode);                                                    \
        } else {                                                                  \
        }                                                                         \
    }
#define THREE                                                                     \
    {                                                                             \
        ifNumberPipe = true;                                                      \
        minus_ttl(&knowNode);                                                     \
        targetNode = getSpecificNode(&knowNode, 0);                               \
        if (ifNumberPipe) {                                                       \
            ttl = tran2number(cmd_3[1][0]);                                       \
            if (targetNode) {                                                     \
                parentNode = getSpecificNode(&knowNode, ttl);                     \
                if (parentNode) {                                                 \
                    addInfo2ExistNode(parentNode, cmd_3, terms_amt_3, cmd_amt_3); \
                    adoptChild(parentNode, targetNode);                           \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_3, terms_amt_3, cmd_amt_3);    \
                    adoptChild(newNode, targetNode);                              \
                }                                                                 \
            } else {                                                              \
                targetNode = getSpecificNode(&knowNode, ttl);                     \
                if (targetNode) {                                                 \
                    addInfo2ExistNode(targetNode, cmd_3, terms_amt_3, cmd_amt_3); \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_3, terms_amt_3, cmd_amt_3);    \
                }                                                                 \
            }                                                                     \
        } else if (targetNode) {                                                  \
            newNode = createNode(&knowNode);                                      \
            addInfo2ExistNode(newNode, cmd_3, terms_amt_3, cmd_amt_3);            \
            adoptChild(newNode, targetNode);                                      \
            bin_func(cmd_3, cmd_amt_3, terms_amt_3, filename, ifNumberPipe,       \
                     newNode);                                                    \
        } else {                                                                  \
        }                                                                         \
    }
#define TWO                                                                       \
    {                                                                             \
        ifNumberPipe = true;                                                      \
        minus_ttl(&knowNode);                                                     \
        targetNode = getSpecificNode(&knowNode, 0);                               \
        if (ifNumberPipe) {                                                       \
            ttl = tran2number(cmd_2[1][0]);                                       \
            if (targetNode) {                                                     \
                parentNode = getSpecificNode(&knowNode, ttl);                     \
                if (parentNode) {                                                 \
                    addInfo2ExistNode(parentNode, cmd_2, terms_amt_2, cmd_amt_2); \
                    adoptChild(parentNode, targetNode);                           \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_2, terms_amt_2, cmd_amt_2);    \
                    adoptChild(newNode, targetNode);                              \
                }                                                                 \
            } else {                                                              \
                targetNode = getSpecificNode(&knowNode, ttl);                     \
                if (targetNode) {                                                 \
                    addInfo2ExistNode(targetNode, cmd_2, terms_amt_2, cmd_amt_2); \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_2, terms_amt_2, cmd_amt_2);    \
                }                                                                 \
            }                                                                     \
        } else if (targetNode) {                                                  \
            newNode = createNode(&knowNode);                                      \
            addInfo2ExistNode(newNode, cmd_2, terms_amt_2, cmd_amt_2);            \
            adoptChild(newNode, targetNode);                                      \
            bin_func(cmd_2, cmd_amt_2, terms_amt_2, filename, ifNumberPipe,       \
                     newNode);                                                    \
        } else {                                                                  \
        }                                                                         \
    }
#define ONE                                                                       \
    {                                                                             \
        ifNumberPipe = true;                                                      \
        minus_ttl(&knowNode);                                                     \
        targetNode = getSpecificNode(&knowNode, 0);                               \
        if (ifNumberPipe) {                                                       \
            ttl = tran2number(cmd_1[1][0]);                                       \
            if (targetNode) {                                                     \
                parentNode = getSpecificNode(&knowNode, ttl);                     \
                if (parentNode) {                                                 \
                    addInfo2ExistNode(parentNode, cmd_1, terms_amt_1, cmd_amt_1); \
                    adoptChild(parentNode, targetNode);                           \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_1, terms_amt_1, cmd_amt_1);    \
                    adoptChild(newNode, targetNode);                              \
                }                                                                 \
            } else {                                                              \
                targetNode = getSpecificNode(&knowNode, ttl);                     \
                if (targetNode) {                                                 \
                    addInfo2ExistNode(targetNode, cmd_1, terms_amt_1, cmd_amt_1); \
                } else {                                                          \
                    newNode = createNode(&knowNode);                              \
                    addInfo2ExistNode(newNode, cmd_1, terms_amt_1, cmd_amt_1);    \
                }                                                                 \
            }                                                                     \
        } else if (targetNode) {                                                  \
            newNode = createNode(&knowNode);                                      \
            addInfo2ExistNode(newNode, cmd_1, terms_amt_1, cmd_amt_1);            \
            adoptChild(newNode, targetNode);                                      \
            bin_func(cmd_1, cmd_amt_1, terms_amt_1, filename, ifNumberPipe,       \
                     newNode);                                                    \
        } else {                                                                  \
        }                                                                         \
    }

static int ID = 0;

typedef struct cmd {
    char terms[TERM_AMT_LIMIT][CHAR_AMT_LIMIT];
    int cnt;
} cmd;

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

typedef struct node {
    struct node *child[CHILD_LIMIT];
    struct node *parent;
    cmd cmds[CMD_PER_NODE_LIMIT];
    int ttl;
    int cmd_cnt;
    int chd_cnt;
    int id;
} node;

typedef struct know_node {
    node *record[NODE_LIMIT];
    int cnt;
} know_node;

node *createNode(know_node *knowNode) {
    node *newNode;
    int *idx;

    newNode = malloc(sizeof(node));
    memset(newNode, 0, sizeof(node));

    idx = &knowNode->cnt;
    knowNode->record[*idx] = newNode;
    (*idx)++;

    newNode->id = ID;
    ID++;

    return newNode;
}

void addInfo2ExistNode(node *Node, char *inputCmd[100][100], int terms_amt[],
                       int cmd_amt) {
    cmd *localCmd;
    int cmd_idx, i;

    cmd_idx = Node->cmd_cnt;
    localCmd = &(Node->cmds[cmd_idx]);
    for (i = 0; i < terms_amt[0]; i++) {
        strcpy(localCmd->terms[i], inputCmd[0][i]);
        localCmd->cnt++;
    }
    //! Maybe write it better.
    if (cmd_amt != 1) {
        Node->ttl = tran2number(inputCmd[1][0]);
    }
    Node->cmd_cnt++;
}

node *getSpecificNode(know_node *knowNode, int ttl) {
    int i;
    for (i = 0; i < knowNode->cnt; i++) {
        if (knowNode->record[i]->ttl == ttl) {
            return knowNode->record[i];
        }
    }
    return NULL;
}

void show_node(know_node *knowNode) {
    int i, j, k;
    node *localNode;
    cmd *localCmd;

    for (i = 0; i < knowNode->cnt; i++) {
        localNode = knowNode->record[i];
        printf("ttl: %d\n", localNode->ttl);
        for (j = 0; j < localNode->cmd_cnt; j++) {
            localCmd = &(localNode->cmds[j]);
            for (k = 0; k < localCmd->cnt; k++) {
                printf("%s\t", localCmd->terms[k]);
            }
            printf("\n");
        }
    }
    printf("*************************\n");
}

node *getParentNode(know_node *knowNode) {
    int i;
    node *localNode;
    for (i = 0; i < knowNode->cnt; i++) {
        localNode = knowNode->record[i];

        if (!localNode->parent) {
            return localNode;
        }
    }
}

void minus_ttl(know_node *knowNode) {
    int i;
    node *localNode;
    for (i = 0; i < knowNode->cnt; i++) {
        localNode = knowNode->record[i];
        if (localNode->ttl != -1) {
            localNode->ttl--;
        }
    }
}

void show_tree(node *Node) {
    int i;
    node *childNode;
    if (Node->chd_cnt) {
        for (i = 0; i < Node->chd_cnt; i++) {
            childNode = Node->child[i];
            show_tree(childNode);
        }
    }
    printf("%d ", Node->id);
}

void adoptChild(node *parent, node *child) {
    int *child_idx;
    child_idx = &(parent->chd_cnt);
    parent->child[*child_idx] = child;
    (*child_idx)++;

    child->parent = parent;
}

void numbered_pipe(node *Node, int cmd_idx) {
    pid_t pid;
    int fds[2], i, j;
    char buf[LENGTH_LIMIT] = {0};
    node *localNode;

    if (!Node->chd_cnt) {
        execlp(Node->cmds[cmd_idx].terms[0], Node->cmds[cmd_idx].terms[0],
               Node->cmds[cmd_idx].terms[1], NULL);
        return;
    }

    if (pipe(fds) == -1) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    localNode = Node->child[cmd_idx];
    for (j = 0; j < localNode->cmd_cnt; j++) {
        pid = fork();
        switch (pid) {
            case -1:
                perror("Error");
                exit(EXIT_FAILURE);
            case 0:
                dup2(fds[1], STDOUT_FILENO);
                close(fds[0]);
                close(fds[1]);
                numbered_pipe(localNode, j);
                exit(EXIT_SUCCESS);
            default:
                wait(NULL);
                break;
        }
    }

    dup2(fds[0], STDIN_FILENO);
    close(fds[0]);
    close(fds[1]);

    execlp(Node->cmds[cmd_idx].terms[0], Node->cmds[cmd_idx].terms[0], NULL);
}

void bin_func(char *terms[100][100], int cmd_amt, int *terms_amt,
              char *filename, bool ifNumberPipe, node *Node) {
    int fd;
    pid_t pid;
    pid = fork();

    switch (pid) {
        case -1:
            perror("Error");
            exit(EXIT_FAILURE);
        case 0:
            // if (*filename) {
            //     fd = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
            //     if (fd == -1) {
            //         perror("Error");
            //         exit(EXIT_FAILURE);
            //     }
            //     dup2(fd, STDOUT_FILENO);
            //     close(fd);
            // }
            if ((Node->id) >= 0) {
                numbered_pipe(Node, 0);
            } else if (cmd_amt == 1) {
                //! It may be wrong, but tackle it later. Only ls is correct.
                if (terms_amt[0] == 1) {
                    execlp(terms[0][0], terms[0][0], NULL);
                } else if (terms_amt[0] > 1) {
                    execlp(terms[0][0], terms[0][0], terms[0][1], NULL);
                }
                perror("Error");
                exit(EXIT_FAILURE);
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

void freeKnowNode(know_node *knowNode) {
    int i;
    for (i = 0; i < knowNode->cnt; i++) {
        free(knowNode->record[i]);
    }
}

int main(void) {
    char *cmd_1[100][100] = {{"bin/number", "test.html"}, {"1"}};
    char *cmd_2[100][100] = {{"bin/number", "test.html"}, {"2"}};
    char *cmd_3[100][100] = {{"bin/removetag", "test.html"}, {"2"}};
    char *cmd_4[100][100] = {{"bin/number"}, {"2"}};
    char *cmd_5[100][100] = {{"bin/number"}, {"5"}};
    char *cmd_6[100][100] = {{"bin/number"}};

    int terms_amt_1[] = {2, 1};
    int terms_amt_2[] = {2, 1};
    int terms_amt_3[] = {2, 1};
    int terms_amt_4[] = {1, 1};
    int terms_amt_5[] = {1, 1};
    int terms_amt_6[] = {1};
    int index, index0, index1;

    int cmd_amt_1 = 2;
    int cmd_amt_2 = 2;
    int cmd_amt_3 = 2;
    int cmd_amt_4 = 2;
    int cmd_amt_5 = 2;
    int cmd_amt_6 = 1;
    int ttl;

    bool ifNumberPipe;
    char *filename = NULL;
    know_node knowNode;
    node *targetNode, *newNode, *parentNode, *currentNode;
    int idx;
    memset(&knowNode, 0, sizeof(know_node));

    // Three
    {
        ifNumberPipe = true;
        minus_ttl(&knowNode);
        targetNode = getSpecificNode(&knowNode, 0);
        if (ifNumberPipe) {
            ttl = tran2number(cmd_3[1][0]);
            if (targetNode) {
                parentNode = getSpecificNode(&knowNode, ttl);
                if (parentNode) {
                    addInfo2ExistNode(parentNode, cmd_3, terms_amt_3, cmd_amt_3);
                    adoptChild(parentNode, targetNode);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_3, terms_amt_3, cmd_amt_3);
                    adoptChild(newNode, targetNode);
                }
            } else {
                targetNode = getSpecificNode(&knowNode, ttl);
                if (targetNode) {
                    addInfo2ExistNode(targetNode, cmd_3, terms_amt_3, cmd_amt_3);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_3, terms_amt_3, cmd_amt_3);
                }
            }
        } else if (targetNode) {
            newNode = createNode(&knowNode);
            addInfo2ExistNode(newNode, cmd_3, terms_amt_3, cmd_amt_3);
            adoptChild(newNode, targetNode);
            bin_func(cmd_3, cmd_amt_3, terms_amt_3, filename, ifNumberPipe, newNode);
        } else {
        }
    }
    // Three
    {
        ifNumberPipe = true;
        minus_ttl(&knowNode);
        targetNode = getSpecificNode(&knowNode, 0);
        if (ifNumberPipe) {
            ttl = tran2number(cmd_3[1][0]);
            if (targetNode) {
                parentNode = getSpecificNode(&knowNode, ttl);
                if (parentNode) {
                    addInfo2ExistNode(parentNode, cmd_3, terms_amt_3, cmd_amt_3);
                    adoptChild(parentNode, targetNode);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_3, terms_amt_3, cmd_amt_3);
                    adoptChild(newNode, targetNode);
                }
            } else {
                targetNode = getSpecificNode(&knowNode, ttl);
                if (targetNode) {
                    addInfo2ExistNode(targetNode, cmd_3, terms_amt_3, cmd_amt_3);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_3, terms_amt_3, cmd_amt_3);
                }
            }
        } else if (targetNode) {
            newNode = createNode(&knowNode);
            addInfo2ExistNode(newNode, cmd_3, terms_amt_3, cmd_amt_3);
            adoptChild(newNode, targetNode);
            bin_func(cmd_3, cmd_amt_3, terms_amt_3, filename, ifNumberPipe, newNode);
        } else {
        }
    }

    // Four
    {
        ifNumberPipe = true;
        minus_ttl(&knowNode);
        targetNode = getSpecificNode(&knowNode, 0);
        if (ifNumberPipe) {
            ttl = tran2number(cmd_4[1][0]);
            if (targetNode) {
                parentNode = getSpecificNode(&knowNode, ttl);
                if (parentNode) {
                    addInfo2ExistNode(parentNode, cmd_4, terms_amt_4, cmd_amt_4);
                    adoptChild(parentNode, targetNode);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_4, terms_amt_4, cmd_amt_4);
                    adoptChild(newNode, targetNode);
                }
            } else {
                targetNode = getSpecificNode(&knowNode, ttl);
                if (targetNode) {
                    addInfo2ExistNode(targetNode, cmd_4, terms_amt_4, cmd_amt_4);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_4, terms_amt_4, cmd_amt_4);
                }
            }
        } else if (targetNode) {
            newNode = createNode(&knowNode);
            addInfo2ExistNode(newNode, cmd_4, terms_amt_4, cmd_amt_4);
            adoptChild(newNode, targetNode);
            bin_func(cmd_4, cmd_amt_4, terms_amt_4, filename, ifNumberPipe, newNode);
        } else {
        }
    }
    // Four
    {
        ifNumberPipe = true;
        minus_ttl(&knowNode);
        targetNode = getSpecificNode(&knowNode, 0);
        if (ifNumberPipe) {
            ttl = tran2number(cmd_4[1][0]);
            if (targetNode) {
                parentNode = getSpecificNode(&knowNode, ttl);
                if (parentNode) {
                    addInfo2ExistNode(parentNode, cmd_4, terms_amt_4, cmd_amt_4);
                    adoptChild(parentNode, targetNode);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_4, terms_amt_4, cmd_amt_4);
                    adoptChild(newNode, targetNode);
                }
            } else {
                targetNode = getSpecificNode(&knowNode, ttl);
                if (targetNode) {
                    addInfo2ExistNode(targetNode, cmd_4, terms_amt_4, cmd_amt_4);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_4, terms_amt_4, cmd_amt_4);
                }
            }
        } else if (targetNode) {
            newNode = createNode(&knowNode);
            addInfo2ExistNode(newNode, cmd_4, terms_amt_4, cmd_amt_4);
            adoptChild(newNode, targetNode);
            bin_func(cmd_4, cmd_amt_4, terms_amt_4, filename, ifNumberPipe, newNode);
        } else {
        }
    }
    // targetNode = getParentNode(&knowNode);
    // show_tree(targetNode);
    // printf("\n");

    // SIX
    {
        ifNumberPipe = false;
        minus_ttl(&knowNode);
        targetNode = getSpecificNode(&knowNode, 0);
        if (ifNumberPipe) {
            ttl = tran2number(cmd_6[1][0]);
            if (targetNode) {
                parentNode = getSpecificNode(&knowNode, ttl);
                if (parentNode) {
                    addInfo2ExistNode(parentNode, cmd_6, terms_amt_6, cmd_amt_6);
                    adoptChild(parentNode, targetNode);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);
                    adoptChild(newNode, targetNode);
                }
            } else {
                targetNode = getSpecificNode(&knowNode, ttl);
                if (targetNode) {
                    addInfo2ExistNode(targetNode, cmd_6, terms_amt_6, cmd_amt_6);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);
                }
            }
        } else if (targetNode) {
            newNode = createNode(&knowNode);
            addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);
            adoptChild(newNode, targetNode);
            bin_func(cmd_6, cmd_amt_6, terms_amt_6, filename, ifNumberPipe, newNode);
        } else {
        }
    }
    // targetNode = getParentNode(&knowNode);
    // show_tree(targetNode);

    // SIX
    {
        ifNumberPipe = false;
        minus_ttl(&knowNode);
        targetNode = getSpecificNode(&knowNode, 0);
        if (ifNumberPipe) {
            ttl = tran2number(cmd_6[1][0]);
            if (targetNode) {
                parentNode = getSpecificNode(&knowNode, ttl);
                if (parentNode) {
                    addInfo2ExistNode(parentNode, cmd_6, terms_amt_6, cmd_amt_6);
                    adoptChild(parentNode, targetNode);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);
                    adoptChild(newNode, targetNode);
                }
            } else {
                targetNode = getSpecificNode(&knowNode, ttl);
                if (targetNode) {
                    addInfo2ExistNode(targetNode, cmd_6, terms_amt_6, cmd_amt_6);
                } else {
                    newNode = createNode(&knowNode);
                    addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);
                }
            }
        } else if (targetNode) {
            newNode = createNode(&knowNode);
            addInfo2ExistNode(newNode, cmd_6, terms_amt_6, cmd_amt_6);
            adoptChild(newNode, targetNode);

            // targetNode = getParentNode(&knowNode);
            // printf("%d\n", targetNode->id);
            // show_tree(targetNode);
            // printf("\n");

            bin_func(cmd_6, cmd_amt_6, terms_amt_6, filename, ifNumberPipe, newNode);
        } else {
        }
    }

    // int i;
    // for (i = 0 ; i < knowNode.cnt ; i++){
    //     printf("%d ", knowNode.record[i]->chd_cnt);
    // }
    // printf("\n");
    // for (i = 0 ; i < knowNode.cnt ; i++){
    //     printf("%d ", knowNode.record[i]->cmd_cnt);
    // }
    // printf("\n");

    // targetNode = getParentNode(&knowNode);
    // show_tree(targetNode);
    // printf("\n");

    freeKnowNode(&knowNode);
}