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
  "printenv", "setenv", "ls", "cat", "removetag", "removetag0", "number"
#define true 1
#define false 0

int main(void) {
  int fds[2];
  if (pipe(fds) == -1) {
    perror("pipe error");
    exit(EXIT_FAILURE);
  }
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("fork error");
    exit(EXIT_FAILURE);
  }
  if (pid == 0) {
    close(fds[0]);  // 子進程關閉讀端 3
    write(fds[1], "hello", 5);
    printf("chld: %d\n", fds[0]);
    exit(EXIT_SUCCESS);
  } else {
    close(fds[1]);  // 父進程關閉寫端 4
    printf("chld2: %d\n", fds[1]);

    if (pipe(fds) == -1) {
        
      perror("pipe error");
      exit(EXIT_FAILURE);
    }
    pid = fork();
    if (pid == 0) {
      close(fds[0]);  // 子進程關閉讀端 4
      write(fds[1], "hello", 5);
      printf("chld3: %d\n", fds[0]);
      exit(EXIT_SUCCESS);
    } else {// 父進程關閉寫端 5
      char buf[50] = {0};
      printf("parent: %d\n", fds[1]);
      read(fds[0], buf, 50);
      printf("receive datas = %s\n", buf);
    }
    return 0;
  }
}