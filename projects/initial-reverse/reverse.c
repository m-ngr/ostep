#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<sys/types.h>
#include<sys/stat.h>

FILE* Fopen(char* path, char* mode){
  FILE* fp = fopen(path, mode);
  if (fp == NULL){
    fprintf(stderr, "reverse: cannot open file '%s'\n", path);
    exit(1);
  }
  return fp;
}

void* Malloc(size_t size){
  void* block = malloc(size);
  if (block == NULL){
    fprintf(stderr, "malloc failed\n");
    exit(1);
  }
  return block;
}

char* Strdup(const char *input){
  char* str = strdup(input);
  if (str == NULL){
    fprintf(stderr, "strdup failed\n"); 
    exit(1);
  }
  return str;
}

typedef struct Node {
  char* data;
  struct Node* next;
} Node;

Node* makeNode(char* data, Node* next){
  Node* node = (Node*) Malloc(sizeof(Node));
  node->data = Strdup(data);
  node->next = next;
  return node;
}

void destroyList(Node* list){
  while(list){
    Node* target = list;
    list = list->next;
    free(target->data);
    free(target);
  }
}

Node* readList(FILE* input){
  Node* list = NULL;
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&line, &len, input)) != -1) {
    list = makeNode(line, list);
  }

  if (line) free(line);

  return list;
}

void writeList(Node* list, FILE* output){
  while(list){
    fprintf(output, "%s", list->data);
    list = list->next;
  }
}

void uniqueFiles(char* inFile, char* outFile){
  struct stat st1, st2;
  if (stat(inFile, &st1) != 0){
    fprintf(stderr, "reverse: cannot open file '%s'\n", inFile);
    exit(1);
  }

  if (stat(outFile, &st2) != 0) return;

  if(st1.st_ino == st2.st_ino && st1.st_dev == st2.st_dev){
    fprintf(stderr, "reverse: input and output file must differ\n");
    exit(1);
  }
}

int main(int argc, char** argv){
  if (argc > 3){
    fprintf(stderr, "usage: reverse <input> <output>\n");
    exit(1);
  }

  if (argc == 3) uniqueFiles(argv[1], argv[2]);

  FILE* input = stdin;
  FILE* output = stdout;

  if (argc > 1) input = Fopen(argv[1], "r");
  if (argc > 2) output = Fopen(argv[2], "w+");

  Node* list = readList(input);
  writeList(list, output);
  destroyList(list);

  if (input != stdin) fclose(input);
  if (output != stdout) fclose(output);
}