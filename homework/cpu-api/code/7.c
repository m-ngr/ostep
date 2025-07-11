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

  if (Fork() == 0){
    close(STDOUT_FILENO);
    printf("Not Printted Text\n");
  }

  return 0;
}