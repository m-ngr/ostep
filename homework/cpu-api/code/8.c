#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int Fork(){
  int res = fork();
  if (res < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  return res;
}

void Pipe(int pipefd[]){
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }
}

int main() {
  int pipefd[2]; // [read:output, write:input]
  pid_t c1, c2;
  
  Pipe(pipefd);
  // First child: writer
  if ((c1 = Fork()) == 0) {
    close(pipefd[0]); // Close unused read end (Pipe Output)
    dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to Pipe Input
    close(pipefd[1]); // Close duplicate fd (STDOUT is used as the write FD for the pipe)
    // Anything printed with printf now goes into the pipe
    printf("Hello from child 1!\n");
    exit(EXIT_SUCCESS);
  }

  // Second child: reader
  if ((c2 = Fork()) == 0) {
    close(pipefd[1]); // Close unused write end (Pipe Input)
    dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to Pipe Output
    close(pipefd[0]);// Close duplicate fd (STDIN is used as the read FD for the pipe)
    // Read from stdin (which is now the pipe)
    char buffer[100];
    fgets(buffer, sizeof(buffer), stdin);
    printf("Child 2 received: %s", buffer);
    exit(EXIT_SUCCESS);
  }
  
  // Parent process: close pipe and wait for children
  close(pipefd[0]);
  close(pipefd[1]);
  waitpid(c1, NULL, 0);
  waitpid(c2, NULL, 0);
  return 0;
}
