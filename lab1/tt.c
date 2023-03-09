#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MSG_SIZE 256

int main() {
  pid_t pid;
  int fds[2], i;
  char buf[100];

  pipe(fds);

  for (i = 0; i < 2; i++) {
    pid = fork();
    if (pid == 0) {
      dup2(fds[1], STDOUT_FILENO);
      close(fds[0]);
      pid = fork();

      pipe(fds);
      if (pid == 0) {

      }
      write(fds[1], "hello", 5);
      exit(EXIT_SUCCESS);
    }
  }
  close(fds[1]);
  for (i = 0; i < 2; i++) {
    memset(buf, 0, 100);
    read(fds[0], buf, 100);
    printf("receive datas = %s\n", buf);
  }
}