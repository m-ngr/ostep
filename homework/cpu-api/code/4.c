#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/wait.h>

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

  if(Fork() == 0){
    fprintf(stdout, "execle: ");
    fflush(stdout);
    execle("/bin/ls","/bin/ls", NULL, __environ);
    exit(EXIT_FAILURE);
  }

  if(Fork() == 0){
    fprintf(stdout, "execv: ");
    fflush(stdout);
    execv("/bin/ls", args);
    exit(EXIT_FAILURE);
  }

  if(Fork() == 0){
    fprintf(stdout, "execve: ");
    fflush(stdout);
    execve("/bin/ls", args, __environ);
  }

  if(Fork() == 0){
    fprintf(stdout, "execl: ");
    fflush(stdout);
    execl("/bin/ls", "/bin/ls", NULL);
  }

  if(Fork() == 0){
    fprintf(stdout, "execl: ");
    fflush(stdout);
    execl("/bin/ls", "/bin/ls", NULL);
  }
 

  
  // File-based (search for file in PATH env)
  execlp("File", "arg0", "arg1", NULL);
  execvp("File", args);
  return 0;
}