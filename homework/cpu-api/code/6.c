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

  if (Fork() != 0){
    int wait_pid = waitpid(-1, NULL, WUNTRACED);
    printf("Parent Wait PID = %d\n", wait_pid);
  }

  return 0;
}