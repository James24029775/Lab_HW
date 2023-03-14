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
int main() {
  char cmd[100] = "printenv";
  int t = system(cmd);
  if (t == 0) {
    printf("YES\n");
  } else {
    printf("NO\n");
  }
}