#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

int main(int argc, char** argv){
  if (argc < 2) {
    printf("wzip: file1 [file2 ...]\n");
    exit(1);
  }

  int current = EOF;
  int last = EOF;
  int count = 0;
  uint8_t buffer[5];

  for(int i = 1; i < argc; ++i){
    FILE* fp = fopen(argv[i], "r");
    if (fp == NULL) exit(1);

    while((current = fgetc(fp)) != EOF){
      if (current == last){
        ++count;
        continue;
      }

      if (count > 0){
        *(int32_t*)buffer = count;
        buffer[4] = (unsigned char)last;
        fwrite(buffer, 5, 1, stdout);
      }

      last = current;
      count = 1;
    }

    fclose(fp);
  }

  if (count > 0){
    *(int32_t*)buffer = count;
    buffer[4] = (unsigned char)last;
    fwrite(buffer, 5, 1, stdout);
  }

  return 0;
}