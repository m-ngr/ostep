#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/wait.h>

int main(){
  int rc = fork();

  if(rc < 0){
    fprintf(stderr, "Failed to fork\n");
    exit(-1);
  }

  if (rc == 0){
    printf("hello\n");
  }else{
    wait(NULL);
    printf("goodbye\n");
  }
  
  return 0;
}