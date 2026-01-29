#include "types.h"
#include "user.h"


int main(int argc, char *argv[])
{
    // 1. Allocate 1 page on the heap using sbrk
  char *addr = sbrk(4096);
  if(addr == (char*)-1){
    printf(1, "sbrk failed\n");
    exit();
  }

  // 2. Write some data to it (should work fine)
  printf(1, "Step 1: Writing 'X' to address %p\n", addr);
  *addr = 'X';
  printf(1, "Step 1: Success\n");

  // 3. Use mprotect to lock it
  printf(1, "Step 2: mprotecting 1 page at %p...\n", addr);
  if(mprotect(addr, 1) < 0){
    printf(1, "mprotect failed!\n");
    exit();
  }

  // 4. Read the data (should work fine even when protected)
  printf(1, "Step 3: Reading data back: %c\n", *addr);
  if(*addr == 'X'){
    printf(1, "Step 3: Success (Read-only access works)\n");
  }

  // 5. Try to write again (should cause a trap/kill)
  printf(1, "Step 4: Attempting to write 'Y' to protected page (expecting crash)...\n");
  *addr = 'Y';

  // If we reach here, protection is NOT working
  printf(1, "ERROR: I'm still alive! mprotect failed to prevent write.\n");
  exit();
}
