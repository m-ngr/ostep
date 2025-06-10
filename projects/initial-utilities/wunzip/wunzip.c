#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

int main(int argc, char** argv){
  if (argc < 2) {
    printf("wunzip: file1 [file2 ...]\n");
    exit(1);
  }

  uint8_t buffer[5];
  
  for(int i = 1; i < argc; ++i){
    FILE* fp = fopen(argv[i], "r");
    if (fp == NULL) exit(1);
    
    while(fread(buffer, 5, 1, fp)){
      int count = *(int32_t*)buffer;
      unsigned char ch = buffer[4];

      for(int i = 0; i < count; ++i) printf("%c",ch);
    }

    fclose(fp);
  }

  return 0;
}