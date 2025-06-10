#include<stdio.h>
#include<stdlib.h>
const int BUFF_SIZE = 2048;

int main(int argc, char** argv){
  char buffer[BUFF_SIZE];
  for(int i = 1; i < argc; ++i){
    char* path = argv[i];

    FILE* fp = fopen(path, "r");
    if (fp == NULL){
      printf("wcat: cannot open file\n");
      exit(1);
    }

    while (1){
      char* str = fgets(buffer, BUFF_SIZE, fp);
      if(str == NULL) break;
      printf("%s", str);
    }

    fclose(fp);
  }
  return 0;
}