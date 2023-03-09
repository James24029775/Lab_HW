#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int fds[2] = {0};

  if (pipe(fds) == -1) {
    perror("Pipe Error.\n");
    exit(EXIT_FAILURE);
  }
  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("Fork Error.\n");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    dup2(fds[1], STDOUT_FILENO);
    close(fds[0]);
    close(fds[1]);
    execlp("ls", "ls", NULL);
    fprintf(stderr, "ls error.\n");
    exit(EXIT_FAILURE);
  }
  dup2(fds[0], STDIN_FILENO);
  close(fds[0]);
  close(fds[1]);
  int fd, max_fd;

  max_fd = getdtablesize();  // 獲取最大文件描述符數量

  for (fd = 0; fd < max_fd; fd++) {
    int flags = fcntl(fd, F_GETFD);
    if (flags != -1) {
      printf("File descriptor %d is open\n", fd);
    }
  }
  execlp("wc", "wc", "-w", NULL);
  fprintf(stderr, "wc error.\n");
  exit(EXIT_FAILURE);
}