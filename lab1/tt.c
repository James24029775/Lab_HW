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
#define OP_PER_NODE_LIMIT 100
#define CMD_PER_OP_LIMIT 100
#define CHILD_LIMIT 100
#define NODE_LIMIT 5000
#define VALID_CMD                                                           \
  {                                                                         \
    "printenv", "setenv", "ls", "cat", "removetag", "removetag0", "number", \
        "exit"                                                              \
  }
#define VALID_CMD_NUM 8
#define bool int
#define IMPOSSIBLE 2147483647
#define true 1
#define false 0

static int ID = 0;

typedef struct cmd {
  char terms[TERM_AMT_LIMIT][CHAR_AMT_LIMIT];
  int cnt;
} cmd;

typedef struct ordinary_pipe_t {
  cmd cmds[CMD_PER_OP_LIMIT];
  int cnt;
} ordinary_pipe_t;

typedef struct number_pipe_t {
  struct number_pipe_t *child[CHILD_LIMIT];
  struct number_pipe_t *parent;
  ordinary_pipe_t ops[OP_PER_NODE_LIMIT];
  int ttl;
  int op_cnt;
  int chd_cnt;
  int id;
} number_pipe_t;

typedef struct know_node {
  number_pipe_t *record[NODE_LIMIT];
  int cnt;
} know_node;

int tran2number(char *);
int split_terms(char *, ordinary_pipe_t *, char *);
int copy_ith_op(ordinary_pipe_t *, ordinary_pipe_t *, int);
void reset_OP(ordinary_pipe_t *);
void show(ordinary_pipe_t *);
void printenv(char[TERM_AMT_LIMIT][CHAR_AMT_LIMIT], int);
void bin_func(ordinary_pipe_t *, char *, number_pipe_t *);
void trim(char *);
void show_node(know_node *);
void minus_ttl(know_node *);
void show_tree(number_pipe_t *);
void adoptChild(number_pipe_t *, number_pipe_t *);
void numbered_pipe(number_pipe_t *, int);
void freeKnowNode(know_node *);
void addInfo2ExistNode(number_pipe_t *, ordinary_pipe_t *, int);
void ordinary_pipe(ordinary_pipe_t *, int);
bool check_valid(ordinary_pipe_t *, char *, bool *, int *);
bool isdigit_myself(char *);
number_pipe_t *createNode(know_node *);
number_pipe_t *getSpecificNode(know_node *, int);
number_pipe_t *getParentNode(know_node *);

int main(void) {
  char input[LENGTH_LIMIT], illegal_term[LENGTH_LIMIT], filename[LENGTH_LIMIT];
  int cmd_amt, terms_amt[LENGTH_LIMIT], ttl, OP_len, np_cnt, i, start_index;
  bool ifNumberPipe, first_flg;
  know_node knowNode;
  number_pipe_t *targetNode, *newNode, *parentNode;
  cmd *localCmd;
  ordinary_pipe_t current_op, tmp_op;

  printf("%% ");
  while (fgets(input, LENGTH_LIMIT, stdin)) {
    // reset
    char *terms[LENGTH_LIMIT][TERM_AMT_LIMIT];
    cmd_amt = OP_len = start_index = 0;
    ifNumberPipe = false;
    memset(terms_amt, 0, LENGTH_LIMIT);
    memset(filename, 0, LENGTH_LIMIT);
    reset_OP(&current_op);
    memset(&current_op, 0, sizeof(ordinary_pipe_t));
    np_cnt = split_terms(input, &current_op, filename);
    memset(input, 0, LENGTH_LIMIT);
    targetNode = newNode = parentNode = NULL;

    minus_ttl(&knowNode);
    targetNode = getSpecificNode(&knowNode, 0);
    if (np_cnt) {
      first_flg = true;
      for (i = 0; i < np_cnt + 1; i++) {
        targetNode = newNode = parentNode = NULL;
        memset(&tmp_op, 0, sizeof(ordinary_pipe_t));
        // show(&current_op);
        start_index = copy_ith_op(&tmp_op, &current_op, start_index);
        if (start_index == -1) {
          break;
        }

        if (!check_valid(&tmp_op, illegal_term, &ifNumberPipe, &OP_len)) {
          printf("Unknown command: [%s].\n", illegal_term);
          printf("%% ");
          continue;
        }

        if (strlen(tmp_op.cmds[OP_len].terms[0]) == 0) {
          ifNumberPipe = false;
        }

        // removeta test.html |2 removetag test.html |1
        // setenv PATH bin
        // removetag test.html |2 removetag test.html |1
        // number |1
        // number
        // number |1 number
        // number test.html | number | removetag |2 removetag test.html |
        // removetag | 1

        //! removetag test.html |2 removetag test.html |1
        if (first_flg) {
          first_flg = false;
        } else {
          minus_ttl(&knowNode);
        }
        targetNode = getSpecificNode(&knowNode, 0);
        if (ifNumberPipe) {
          ttl = tran2number(tmp_op.cmds[OP_len].terms[0]);
          if (targetNode) {
            parentNode = getSpecificNode(&knowNode, ttl);
            if (parentNode) {
              addInfo2ExistNode(parentNode, &tmp_op, OP_len);
              adoptChild(parentNode, targetNode);
            } else {
              newNode = createNode(&knowNode);
              addInfo2ExistNode(newNode, &tmp_op, OP_len);
              adoptChild(newNode, targetNode);
            }
          } else {
            targetNode = getSpecificNode(&knowNode, ttl);
            if (targetNode) {
              addInfo2ExistNode(targetNode, &tmp_op, OP_len);
            } else {
              newNode = createNode(&knowNode);
              addInfo2ExistNode(newNode, &tmp_op, OP_len);
            }
          }
        } else if (targetNode) {
          newNode = createNode(&knowNode);
          addInfo2ExistNode(newNode, &current_op, OP_len);
          adoptChild(newNode, targetNode);
          bin_func(&current_op, filename, newNode);
        }
        // show_node(&knowNode);
      }
    } else {
      if (!check_valid(&current_op, illegal_term, &ifNumberPipe, &OP_len)) {
        printf("Unknown command: [%s].\n", illegal_term);
        printf("%% ");
        continue;
      }
      if (targetNode) {
        newNode = createNode(&knowNode);
        addInfo2ExistNode(newNode, &current_op, OP_len);
        adoptChild(newNode, targetNode);
        bin_func(&current_op, filename, newNode);
      } else {
        localCmd = &(current_op.cmds[0]);
        if (!strcmp(localCmd->terms[0], "printenv")) {
          printenv(localCmd->terms, localCmd->cnt);
        } else if (!strcmp(localCmd->terms[0], "setenv")) {
          setenv(localCmd->terms[1], localCmd->terms[2], 1);
        } else if (!strcmp(localCmd->terms[0], "exit")) {
          exit(EXIT_SUCCESS);
        } else {
          bin_func(&current_op, filename, newNode);
        }
      }
    }

    printf("%% ");
  }
}

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

// Return the number of number_pipe_t
int split_terms(char *input, ordinary_pipe_t *op, char *filename) {
  char d[10];
  char *p, *commands;
  char *command[LENGTH_LIMIT];
  char local_input[LENGTH_LIMIT];
  cmd *localCmd;
  int i, j, k, np_cnt;

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
  op->cnt = i;

  i = 0;
  memset(d, 0, 10);
  strcpy(d, "  ");

  //! I just can't figure out why the last time array value would be
  //! kept.!!!!!!!!!!!!!!!!!!!!!!!
  char **t = command;
  while (*t) {
    j = 0;
    p = strtok(*t, d);
    localCmd = &(op->cmds[i]);
    while (p != NULL) {
      // I don't surely know why replace assign for strcpy would be wrong.
      strcpy(localCmd->terms[j++], p);
      localCmd->cnt++;
      p = strtok(NULL, d);
    }
    i++;
    t++;
  }

  np_cnt = 0;
  for (i = 0; i < op->cnt; i++) {
    if (isdigit_myself(op->cmds[i].terms[0])) {
      np_cnt++;
    }
  }
  return np_cnt;
}

// Return end index
int copy_ith_op(ordinary_pipe_t *tmp_op, ordinary_pipe_t *current_op,
                int start_index) {
  int i, j;
  bool first_flg;
  cmd *tmpCmd, *localCmd;

  first_flg = true;
  if (start_index == 0) {
    for (i = 0; i < current_op->cnt; i++) {
      localCmd = &(current_op->cmds[i]);
      tmpCmd = &(tmp_op->cmds[i]);
      if (isdigit_myself(localCmd->terms[0]) && !first_flg) {
        strcpy(tmpCmd->terms[0], localCmd->terms[0]);
        tmpCmd->cnt++;
        tmp_op->cnt++;
        return i;
      }
      for (j = 0; j < localCmd->cnt; j++) {
        strcpy(tmpCmd->terms[j], localCmd->terms[j]);
        tmpCmd->cnt++;
      }
      first_flg = false;
      tmp_op->cnt++;
    }
  } else {
    for (i = start_index; i < current_op->cnt; i++) {
      localCmd = &(current_op->cmds[i]);
      tmpCmd = &(tmp_op->cmds[i - start_index]);

      if (strlen(localCmd->terms[1]) == 0 && first_flg) {
        return -1;
      }

      if (isdigit_myself(localCmd->terms[0]) && !first_flg) {
        strcpy(tmpCmd->terms[0], localCmd->terms[0]);
        tmpCmd->cnt++;
        tmp_op->cnt++;
        return i;
      }
      if (first_flg) {
        for (j = 0; j < localCmd->cnt - 1; j++) {
          strcpy(tmpCmd->terms[j], localCmd->terms[j + 1]);
          tmpCmd->cnt++;
        }
      } else {
        for (j = 0; j < localCmd->cnt; j++) {
          strcpy(tmpCmd->terms[j], localCmd->terms[j]);
          tmpCmd->cnt++;
        }
      }
      // I just can not figure out why current_op save last time records, but i
      // have already do memset or reset_OP. It took me 4 hours but was still
      // unsolved.
      // show(current_op);
      first_flg = false;
      tmp_op->cnt++;
    }
  }
}

void reset_OP(ordinary_pipe_t *current_op) {
  cmd *localCmd;
  int i, j;
  for (i = 0; i < current_op->cnt; i++) {
    localCmd = &(current_op->cmds[i]);
    for (j = 0; j < localCmd->cnt; j++) {
      memset(localCmd->terms[j], 0, sizeof(localCmd->terms[j]));
    }
    localCmd->cnt = 0;
  }
  current_op->cnt = 0;
}

void show(ordinary_pipe_t *localOP) {
  int x, y;
  cmd *localCmd;
  printf("Commands amount: %d\n", localOP->cnt);

  for (x = 0; x < 3; x++) {
    localCmd = &(localOP->cmds[x]);
    printf("%d\t", localCmd->cnt);
    for (y = 0; y < localCmd->cnt; y++) {
      printf("%s\t", localCmd->terms[y]);
    }
    printf("\n");
  }
  printf("\n");
}

void printenv(char terms[TERM_AMT_LIMIT][CHAR_AMT_LIMIT], int cmd_amt) {
  if (cmd_amt == 1) {
    printf("Usage: printenv [var]");
  } else {
    char *s = getenv(*(terms + 1));
    if (s) {
      printf("%s\n", s);
    }
  }
}

void bin_func(ordinary_pipe_t *localOP, char *filename, number_pipe_t *Node) {
  int fd;
  cmd *localCmd;
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

      if (Node) {
        numbered_pipe(Node, 0);
      } else if (localOP->cnt == 1) {
        //! It may be wrong, but tackle it later. Only ls is correct.
        localCmd = &(localOP->cmds[0]);
        if (localCmd->cnt == 1) {
          execlp(localCmd->terms[0], localCmd->terms[0], NULL);
        } else if (localCmd->cnt > 1) {
          execlp(localCmd->terms[0], localCmd->terms[0], localCmd->terms[1],
                 NULL);
        }
        perror("Error");
        exit(EXIT_FAILURE);
      } else {
        // ls | bin will crush.
        ordinary_pipe(localOP, localOP->cnt - 1);
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

void ordinary_pipe(ordinary_pipe_t *op, int index) {
  if (index == 0) {
    if (op->cmds[index].cnt == 1) {
      execlp(op->cmds[index].terms[0], op->cmds[index].terms[0], NULL);
    } else if (op->cmds[index].cnt == 2) {
      execlp(op->cmds[index].terms[0], op->cmds[index].terms[0],
             op->cmds[index].terms[1], NULL);
    }
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
      ordinary_pipe(op, index - 1);
      perror("Error");
      exit(EXIT_FAILURE);
    default:
      dup2(fds[0], STDIN_FILENO);
      close(fds[0]);
      close(fds[1]);
      execlp(op->cmds[index].terms[0], op->cmds[index].terms[0], NULL);
      perror("Error");
      exit(EXIT_FAILURE);
  }
}

void minus_ttl(know_node *knowNode) {
  int i;
  number_pipe_t *localNode;
  for (i = 0; i < knowNode->cnt; i++) {
    localNode = knowNode->record[i];
    if (localNode->ttl != -1) {
      localNode->ttl--;
    }
  }
}

void show_tree(number_pipe_t *Node) {
  int i;
  number_pipe_t *childNode;
  if (Node->chd_cnt) {
    for (i = 0; i < Node->chd_cnt; i++) {
      childNode = Node->child[i];
      show_tree(childNode);
    }
  }
  printf("%d ", Node->id);
}

void adoptChild(number_pipe_t *parent, number_pipe_t *child) {
  int *child_idx;
  child_idx = &(parent->chd_cnt);
  parent->child[*child_idx] = child;
  (*child_idx)++;

  child->parent = parent;
}

void numbered_pipe(number_pipe_t *Node, int op_idx) {
  pid_t pid;
  int fds[2], i, j;
  number_pipe_t *localNode;
  cmd *localCmd;
  ordinary_pipe_t *op;

  if (!Node->chd_cnt) {
    op = &(Node->ops[op_idx]);
    ordinary_pipe(op, op->cnt - 1);
    return;
  }

  if (pipe(fds) == -1) {
    perror("Error");
    exit(EXIT_FAILURE);
  }

  localNode = Node->child[op_idx];

  for (j = 0; j < localNode->op_cnt; j++) {
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
  op = &(Node->ops[op_idx]);
  // ordinary_pipe(op, op->cnt - 1);
  execlp(Node->ops[op_idx].cmds[0].terms[0], Node->ops[op_idx].cmds[0].terms[0],
         NULL);
}

void freeKnowNode(know_node *knowNode) {
  int i;
  for (i = 0; i < knowNode->cnt; i++) {
    free(knowNode->record[i]);
  }
}

void addInfo2ExistNode(number_pipe_t *Node, ordinary_pipe_t *current_op,
                       int op_len) {
  ordinary_pipe_t *localOP;
  cmd *localCmd, *currentCmd;
  int op_idx, i, j;

  op_idx = Node->op_cnt;
  localOP = &(Node->ops[op_idx]);
  for (i = 0; i < op_len; i++) {
    localCmd = &(localOP->cmds[i]);
    currentCmd = &(current_op->cmds[i]);
    for (j = 0; j < currentCmd->cnt; j++) {
      strcpy(localCmd->terms[j], currentCmd->terms[j]);
      localCmd->cnt++;
    }
    localOP->cnt++;
  }
  //! Maybe write it better.
  if (current_op->cnt != 1) {
    Node->ttl = tran2number(current_op->cmds[op_len].terms[0]);
  }
  Node->op_cnt++;
}

void show_node(know_node *knowNode) {
  int i, j, k, z;
  number_pipe_t *localNode;
  ordinary_pipe_t *localOP;
  cmd *localCmd;

  for (i = 0; i < knowNode->cnt; i++) {
    localNode = knowNode->record[i];
    printf("ttl: %d\n", localNode->ttl);
    for (j = 0; j < localNode->op_cnt; j++) {
      localOP = &(localNode->ops[j]);
      for (k = 0; k < localOP->cnt; k++) {
        localCmd = &(localOP->cmds[k]);
        for (z = 0; z < localCmd->cnt; z++) {
          printf("%s\t", localCmd->terms[z]);
        }
        printf("|\t");
      }
      printf("\n");
    }
  }
  printf("*************************\n");
}

bool check_valid(ordinary_pipe_t *op, char *illegal_term, bool *ifNumberPipe,
                 int *OP_len) {
  int i, j;
  bool legal_flg;
  char *valid_cmd[VALID_CMD_NUM] = VALID_CMD;
  cmd *localCmd;

  for (i = 0; i < op->cnt; i++) {
    legal_flg = false;
    localCmd = &(op->cmds[i]);
    for (j = 0; j < VALID_CMD_NUM; j++) {
      *OP_len = i + 1;
      if (!strcmp(localCmd->terms[0], valid_cmd[j])) {
        legal_flg = true;
        break;
      }
      if (isdigit_myself(localCmd->terms[0]) &&
          tran2number(localCmd->terms[0]) != IMPOSSIBLE) {
        legal_flg = true;
        *OP_len = i;
        *ifNumberPipe = true;
        break;
      }
    }
    if (!legal_flg) {
      strcpy(illegal_term, localCmd->terms[0]);
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

number_pipe_t *createNode(know_node *knowNode) {
  number_pipe_t *newNode;
  int *idx;

  newNode = malloc(sizeof(number_pipe_t));
  memset(newNode, 0, sizeof(number_pipe_t));

  idx = &knowNode->cnt;
  knowNode->record[*idx] = newNode;
  (*idx)++;

  newNode->id = ID;
  ID++;

  return newNode;
}

number_pipe_t *getSpecificNode(know_node *knowNode, int ttl) {
  int i;
  for (i = 0; i < knowNode->cnt; i++) {
    if (knowNode->record[i]->ttl == ttl) {
      return knowNode->record[i];
    }
  }
  return NULL;
}

number_pipe_t *getParentNode(know_node *knowNode) {
  int i;
  number_pipe_t *localNode;
  for (i = 0; i < knowNode->cnt; i++) {
    localNode = knowNode->record[i];

    if (!localNode->parent) {
      return localNode;
    }
  }
}