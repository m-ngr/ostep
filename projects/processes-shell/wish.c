#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#define PROMPT "wish>"

char** paths = NULL;
int paths_count = 0;

void free_sarr(char** sarr, int len) {
  if (sarr == NULL) return;
  for (int i = 0; i < len; i++) {
    free(sarr[i]);
  }
  free(sarr);
}

void set_paths(char** input, int count) {
  free_sarr(paths, paths_count);
  paths = malloc(count * sizeof(char*));

  for (int i = 0; i < count; ++i) {
    paths[i] = strdup(input[i]);
  }

  paths_count = count;
}

void print_error() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

char** tokenize(const char* input, const char* delims, int* count) {
  char* copy = strdup(input);
  if (!copy) return NULL;

  int capacity = 10;
  int size = 0;
  char** tokens = malloc(capacity * sizeof(char*));
  if (!tokens) {
    free(copy);
    return NULL;
  }
  char* tok = strtok(copy, delims);
  while (tok) {
    if (size >= capacity) {
      capacity *= 2;
      tokens = realloc(tokens, capacity * sizeof(char*));
      if (!tokens) {
        free(copy);
        return NULL;
      }
    }
    tokens[size++] = strdup(tok);
    tok = strtok(NULL, delims);
  }

  free(copy);
  *count = size;
  return tokens;
}

void redirect(char* out_file) {
  int fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  if (fd < 0) {
    print_error();
    exit(1);
  }
    
  if (dup2(fd, STDOUT_FILENO) < 0) {
    print_error();
    exit(1);
  }

  if (dup2(fd, STDERR_FILENO) < 0) {
    print_error();
    exit(1);
  }

  close(fd);
}

int execute_binary(char* path, int argc, char* argv[], char* out_file) {
  int pid = fork();
  
  if (pid == 0) {
    if (out_file) redirect(out_file);
    execv(path, argv);
    print_error();
  }

  return pid;
}

char* get_binary(char* name) {
  char* exe = NULL;

  for (int i = 0; i < paths_count; ++i) {
    int len = strlen(paths[i]) + strlen(name) + 2;
    char* file = malloc(len);
    if (!file) return NULL;
    snprintf(file, len, "%s/%s", paths[i], name);
    if (access(file, X_OK) == 0) {
      exe = file;
      break;
    }

    free(file);
  }

  return exe;
}

int run_binary(int argc, char** argv, char* out_file) {
  char* exe = get_binary(argv[0]);
  if (exe == NULL) {
    print_error();
    return -1;
  }
  int pid = execute_binary(exe, argc, argv, out_file);
  free(exe);
  return pid;
}

void exit_command(int argc, char** argv) {
  if (argc == 1) exit(0);
  print_error();
}

void cd_command(int argc, char** argv) {
  if (argc != 2) {
    print_error();
    return;
  }
  int rc = chdir(argv[1]);
  if (rc != 0) print_error();
}

void path_command(int argc, char** argv) { 
  set_paths(&argv[1], argc - 1);
}

void debug_command(int argc, char** argv) {
  printf("Paths: %d\n", paths_count);
  for (int i = 0; i < paths_count; ++i) {
    printf("\t%s\n", paths[i]);
  }
}

int builtin_command(int argc, char** argv) {
  if (strcasecmp(argv[0], "exit") == 0) {
    exit_command(argc, argv);
  } else if (strcasecmp(argv[0], "cd") == 0) {
    cd_command(argc, argv);
  } else if (strcasecmp(argv[0], "path") == 0) {
    path_command(argc, argv);
  } else if (strcasecmp(argv[0], "debug") == 0) {
    debug_command(argc, argv);
  } else {
    return 0;
  }

  return 1;
}

int count_char(const char* str, char c) {
  int count = 0;
  for (const char *p = str; *p; p++) {
    if (*p == c) count++;
  }
  return count;
}

int count_tokens(char* input, char* delims) {
  // inefficient: lazy to write it.
  int count = 0;
  char** toks = tokenize(input, delims, &count);
  free_sarr(toks, count);
  return count;
}

char *trim(char *str) {
  char *end;

  while (isspace((unsigned char)*str)) str++;

  if (*str == 0) return str;

  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) end--;
  
  *(end + 1) = '\0';

  return str;
}

char** parse_command(char* command){
  int count = count_char(command, '>');
  if (count > 1) return NULL;

  if (count == 0){
    char** res = malloc(sizeof(char*) * 2);
    res[0] = command;
    res[1] = NULL;
    return res;
  } 
  
  int parts_count = 0;
  char** parts = tokenize(command, ">", &parts_count);

  if (parts_count != 2){
    free_sarr(parts, parts_count);
    return NULL;
  }

  int files_count = count_tokens(parts[1], "\t\n ");

  if (files_count != 1){
    free_sarr(parts, parts_count);
    return NULL;
  }

  parts[1] = trim(parts[1]);

  return parts;
}

int run_command(char* command) {
  char** cmd = parse_command(command);

  if (cmd == NULL){
    print_error();
    return -1;
  }

  int rc = -1;
  int argc = 0;
  char** argv = tokenize(cmd[0], "\t\n ", &argc);

  if (argc != 0) {
    if (builtin_command(argc, argv)) {
      rc = 0;
    } else {
      rc = run_binary(argc, argv, cmd[1]);
    }
  }

  free_sarr(argv, argc);
  // free_sarr(cmd, 2);
  return rc;
}

void await(int pids[], int len) {
  for(int i = 0; i < len; ++i) {
    if (pids[i] <= 0) continue;
    waitpid(pids[i], NULL, 0);
  }
}

void repl(FILE* stream, int interactive) {
  char* line = NULL;
  size_t len = 0;
  ssize_t n;

  if (interactive) printf("%s ", PROMPT);
  while ((n = getline(&line, &len, stream)) != EOF) {
    int cmd_count = 0;
    char** cmds = tokenize(line, "&", &cmd_count);
    if (cmd_count == 0) continue;

    int* pids = (int*) malloc(sizeof(int) * cmd_count);

    for(int i = 0; i < cmd_count; ++i){
      pids[i] = run_command(cmds[i]);
    }

    await(pids, cmd_count);

    free(pids);
    free_sarr(cmds, cmd_count);
    fflush(stdout);
    if (interactive) printf("%s ", PROMPT);
  }

  free(line);
}

int main(int argc, char** argv) {
  if (argc > 2) {
    print_error();
    exit(1);
  }

  FILE* stream = stdin;
  int interactive = 1;

  if (argc == 2) {
    stream = fopen(argv[1], "r");
    if (!stream) {
      print_error();
      exit(1);
    }

    interactive = 0;
  }

  char* def_paths[] = {"/bin", "/usr/bin"};
  set_paths(def_paths, 2);

  repl(stream, interactive);

  return 0;
}