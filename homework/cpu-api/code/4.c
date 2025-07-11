#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/wait.h>

extern char **environ;

int Fork(){
  int rc = fork();

  if(rc < 0){
    fprintf(stderr, "Failed to fork\n");
    exit(-1);
  }

  return rc;
}

int main(){
  char* args[] = {"/bin/ls", NULL};

  if(Fork() == 0){
    fprintf(stdout, "execl: ");
    fflush(stdout);
    execl("/bin/ls", "/bin/ls", NULL);
    exit(EXIT_FAILURE);
  }

  wait(NULL);

  if(Fork() == 0){
    fprintf(stdout, "execle: ");
    fflush(stdout);
    execle("/bin/ls","/bin/ls", NULL, environ);
    exit(EXIT_FAILURE);
  }

  wait(NULL);

  if(Fork() == 0){
    fprintf(stdout, "execv: ");
    fflush(stdout);
    execv("/bin/ls", args);
    exit(EXIT_FAILURE);
  }

  wait(NULL);

  if(Fork() == 0){
    fprintf(stdout, "execve: ");
    fflush(stdout);
    execve("/bin/ls", args, environ);
  }

  wait(NULL);

  if(Fork() == 0){
    fprintf(stdout, "execlp: ");
    fflush(stdout);
    setenv("PATH", "/bin", 1);
    execlp("ls", "ls", NULL);
  }

  wait(NULL);

  if(Fork() == 0){
    fprintf(stdout, "execvp: ");
    fflush(stdout);
    setenv("PATH", "/bin", 1);
    execvp("ls", args);
  }

  wait(NULL);

  return 0;
}