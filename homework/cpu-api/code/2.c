#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <fcntl.h>

int main(){
  int fd = open("./x.txt", O_WRONLY | O_RDONLY  | O_CREAT);

  int rc = fork();

  if(rc < 0){
    fprintf(stderr, "Failed to fork\n");
    exit(-1);
  }

  if (rc == 0){
    write(fd, "child\n",6);
    close(fd);
  }else{
    write(fd, "parent\n",7);
    close(fd);
  }
  
  return 0;
}