#include "types.h"
#include "user.h"

#define PGSIZE 4096

void test_local_toggle() {
  printf(1, "\n--- Test 1: Local Toggle ---\n");
  char *addr = sbrk(PGSIZE);
  
  printf(1, "Writing 'A'...");
  *addr = 'A';
  printf(1, "OK\n");

  mprotect(addr, 1);
  printf(1, "Protected. Reading value: %c (OK)\n", *addr);

  munprotect(addr, 1);
  printf(1, "Unprotected. Writing 'B'...");
  *addr = 'B';
  printf(1, "OK\n");
}

void test_fork_inheritance() {
  printf(1, "\n--- Test 2: Fork Inheritance ---\n");
  char *addr = sbrk(PGSIZE);
  *addr = 'P'; // Parent value

  printf(1, "Parent mprotecting %p...\n", addr);
  mprotect(addr, 1);

  int pid = fork();
  if(pid < 0){
    printf(1, "Fork failed\n");
    exit();
  }

  if(pid == 0){
    // CHILD
    printf(1, "Child: Reading inherited value: %c\n", *addr);
    printf(1, "Child: Attempting illegal write (Should crash child)...\n");
    *addr = 'C'; 
    printf(1, "ERROR: Child survived illegal write!\n");
    exit();
  } else {
    // PARENT
    wait();
    printf(1, "Parent: Child has finished (likely crashed).\n");
    printf(1, "Parent: Attempting illegal write (Should crash parent)...\n");
    *addr = 'P';
    printf(1, "ERROR: Parent survived illegal write!\n");
  }
}

int main(int argc, char *argv[]) {
  test_local_toggle();
  test_fork_inheritance();
  exit();
}