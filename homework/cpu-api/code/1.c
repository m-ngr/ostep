#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main(){
  int x = 100;

  int rc = fork();

  if(rc < 0){
    fprintf(stderr, "Failed to fork\n");
    exit(-1);
  }

  if (rc == 0){
    x = 50;
    printf("[Child] Value of X = %d\n", x);
  }else{
    x = 20;
    printf("[Parent] Value of X = %d\n", x);
  }

  return 0;
}