# Solution

## User-Level Interface

in `user.h` file, define the user-level function call interface:

```c user.h
int getreadcount(void);
```

---

## System function

in `sysfile.c` file, we will add a system function that must conform to the following interface: `int sys_{name}(void)`. A function that starts with `sys_` takes no arguments and return an int.

Also, we will update the `sys_read` function to increase a counter `readcount` when called

```c sysfile.c

int readcount = 0;

int sys_read(void) {
  readcount++;
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

int sys_getreadcount(void) {
  return readcount;
}

```

---

## Assign a number for the system call

in `syscall.h`, add a define for the system call in the following format `SYS_{name}`

```c syscall.h
#define SYS_getreadcount  22
```

then add it to the assembly code of syscalls in `usys.S` file, in the following format: `SYSCALL({name})`

```S usys.S
SYSCALL(getreadcount)
```

---

## Link the system call number with system function

in the `syscall.c` file, add the system function address as entry in the `syscalls[]` using it system call number as index.

```c syscall.c
// define function signature for the linker
extern int sys_getreadcount(void);

static int (*syscalls[])(void) = {
  [SYS_fork]    sys_fork,
  [SYS_exit]    sys_exit,
  //...
  [SYS_getreadcount] sys_getreadcount, // add the function entry
}
```
