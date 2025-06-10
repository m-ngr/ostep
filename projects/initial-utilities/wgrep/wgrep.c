#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>

void printMatch(char* term, FILE* fp){
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&line, &len, fp)) != -1) {
    if (strstr(line, term)) printf("%s",line);
  }

  if (line != NULL) free(line);
}

int main(int argc, char** argv){
  if (argc < 2){
    printf("wgrep: searchterm [file ...]\n");
    exit(1);
  }

  char* term = argv[1];


  for(int i = 2; i < argc; ++i ){
    FILE* fp = fopen(argv[i], "r");
    if (fp == NULL){
      printf("wgrep: cannot open file\n");
      exit(1);
    }
    printMatch(term, fp);
    fclose(fp);
  }

  if(argc == 2){
    printMatch(term, stdin);
  }

  return 0;
}

