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

void scan(char input[1000], int *ith) {
  int idx, i;
  idx = 0;
  *ith = -1;
  for (i = 1; i < strlen(input); i++) {
    if (isdigit(input[i])) {
      if (input[i - 1] == '|') {
        idx++;
      } else if (input[i - 1] == '!') {
        *ith = idx;
        idx++;
        input[i - 1] = '|';
      }
    }
  }
}

int main(void) {
	return 0;
  // int ith;
  // char input[LENGTH_LIMIT], illegal_term[LENGTH_LIMIT], filename[LENGTH_LIMIT];
  // ordinary_pipe_t current_op;
  // char input[1000] = "removetag0 test.html !2 number test.html |1\n";
  // scan(input, &ith);
  // memset(&current_op, 0, sizeof(ordinary_pipe_t));
  // memset(filename, 0, LENGTH_LIMIT);
  // np_cnt = split_terms(input, &current_op, filename);
  // printf("%d %s\n", ith, input);
}