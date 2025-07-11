# CPU API Homework

## Instructions

There are now two simulators related to this chapter. The first,
`fork.py`, is a simple tool to show what a process tree looks like
when processes are created and destroyed. Read more about it
[here](README-fork.md).

The second is a program, `generator.py`, that creates real C programs
that use `fork()`, `wait()`, and `exit()` to show how `fork` works in
running programs. Read more about it [here](README-generator.md).

## Simulation Questions

1.  Run `./fork.py -s 10` and see which actions are taken. Can you predict what the process tree looks like at each step?

**Answer:**

| Action    | Process Tree |
| --------- | ------------ |
| start     | a            |
| a forks b | a -> (b)     |
| a forks c | a -> (b,c)   |
| c EXITS   | a -> (b)     |
| a forks d | a -> (b,d)   |
| a forks e | a -> (b,d,e) |

---

2. One control the simulator gives you is the fork percentage, controlled by the -f flag. The higher it is, the more likely the next action is a fork; the lower it is, the more likely the action is an exit. Run the simulator with a large number of actions (e.g., -a 100) and vary the fork percentage from 0.1 to 0.9. What do you think the resulting final process trees will look like as the percentage changes? Check your answer with -c.

**Answer:** Higher fork percentage means larger tree (depth x width).

---

3. Now, switch the output by using the -t flag (e.g., run `./fork.py -t`). Given a set of process trees, can you tell which actions were taken?

**Answer:**

| Action    | Process Tree        |
| --------- | ------------------- |
| start     | a                   |
| a forks b | a -> (b)            |
| a forks c | a -> (b,c)          |
| b forks d | a -> (b->(d),c)     |
| b forks e | a -> (b->(d,e),c)   |
| b forks f | a -> (b->(d,e,f),c) |
| a forks d | a -> (b,d)          |
| a forks e | a -> (b,d,e)        |

---

4. One interesting thing to note is what happens when a child exits; what happens to its children in the process tree? To study this, let’s create a specific example: `./fork.py -A a+b,b+c,c+d,c+e,c-`. This example has process ’a’ create ’b’, which in turn creates ’c’, which then creates ’d’ and ’e’. However, then, ’c’ exits. What do you think the process tree should like after the exit? What if you use the -R flag? Learn more about what happens to orphaned processes on your own to add more context.

**Answer:** Orphaned Processes are being adopted by the init process

---

5. One last flag to explore is the -F flag, which skips intermediate steps and only asks to fill in the final process tree. Run `./fork.py -F` and see if you can write down the final tree by looking at the series of actions generated. Use different random seeds to try this a few times.

**Answer:**

- Running `./fork.py -F -s 5`
- Final Process Tree (a -> d)

---

6. Finally, use both -t and -F together. This shows the final process tree, but then asks you to fill in the actions that took place. By looking at the tree, can you determine the exact actions that took place? In which cases can you tell? In which can’t you tell? Try some different random seeds to delve into this question.

### Random S = 1

- Run `./fork.py -t -F -s 1`

- Final Process Tree:

```md
a
├── b
├── e
└── d
```

**Answer:**

Action: a fork b
Action: a fork c
Action: c fork d
Action: a fork e
Action: c Exit

### Random S = 2

- Run `./fork.py -t -F -s 1`

- Final Process Tree:

```md
a
├── c
│ └── d
└── e
```

**Answer:**

Action: a fork b
Action: a fork c
Action: c fork d
Action: a fork e
Action: b Exit

---

## Code Questions

1. Write a program that calls fork(). Before calling fork(), have the main process access a variable (e.g., x) and set its value to some- thing (e.g., 100). What value is the variable in the child process? What happens to the variable when both the child and parent change the value of x?

**Answer:**

- the value of x is 100 in both parent and child
- each process has its own private memory so each process will have its own value of x after change

---

2. Write a program that opens a file (with the open() system call) and then calls fork() to create a new process. Can both the child and parent access the file descriptor returned by open()? What happens when they are writing to the file concurrently, i.e., at the same time?

**Answer:**

- the parent and the child can write to the same file

- if text is too long it might be interleaved due to context switching

---

3. Write another program using fork(). The child process should print “hello”; the parent process should print “goodbye”. You should try to ensure that the child process always prints first; can you do this without calling wait() in the parent?

**Answer:**

- code at [./code/3.c](./code/3.c)
- we could use sleep to delay the excution of the parent but is not reliable

---

4. Write a program that calls fork() and then calls some form of exec() to run the program `/bin/ls`. See if you can try all of the variants of exec(), including (on Linux) execl(), execle(), execlp(), execv(), execvp(), and execvpe(). Why do you think there are so many variants of the same basic call?

**Answer:**

- code at: [./code/4.c](./code/4.c)
- simple explaination

```c
  char* args[3];
  args[0] = "arg0";
  args[1] = "arg1";
  args[2] = NULL;
  char* env[3];
  env[0] = "env0=val0";
  env[1] = "env1=val1";
  env[2] = NULL;

  // Path-based
  execl("Path","arg0", "arg1", NULL);
  execle("Path","arg0", "arg1", NULL, env);
  execv("Path", args);
  execve("Path", args, env);
  // File-based (search for file in PATH env)
  execlp("File", "arg0", "arg1", NULL);
  execvp("File", args);
```

| call     | args                            | env                             | Use `PATH` |
| -------- | ------------------------------- | ------------------------------- | ---------- |
| execl()  | null-terminated string arg list | inherited from caller `environ` | no         |
| execle() | null-terminated string arg list | null-terminated string array    | no         |
| execv()  | null-terminated string array    | inherited from caller `environ` | no         |
| execve() | null-terminated string array    | null-terminated string array    | no         |
| execlp() | null-terminated string arg list | inherited from caller `environ` | yes        |
| execvp() | null-terminated string array    | inherited from caller `environ` | yes        |

---

5. Now write a program that uses wait() to wait for the child process to finish in the parent. What does wait() return? What happens if you use wait() in the child?

**Answer:**

- in parent: it returns the `pid` of the child process that finished excution.
- in child: it returns -1 to indiciate no child processes to wait on

---

6. Write a slight modification of the previous program, this time using waitpid() instead of wait(). When would waitpid() be useful?

**Answer:**

waitpid() can be useful for:

- Non-blocking wait: check exited childs without blocking the parent execution
- Waiting for specific Process
- Waiting for any process from a specific group

---

7. Write a program that creates a child process, and then in the child closes standard output (STDOUT FILENO). What happens if the child calls printf() to print some output after closing the descriptor?

**Answer:**
it doesn't print anything to the terminal

---

8. Write a program that creates two children, and connects the standard output of one to the standard input of the other, using the pipe() system call.

**Answer:**

- See code at [./code/8.c](./code/8.c)
- The idea is simple:
  1. open a new pipe in the parent process
  2. fork 2 childs
  3. in child 1 (the writer):
     a. close pipe read end
     b. redirect stdout to pipe write end
  4. in child 2 (the reader):
     a. close the pipe write end
     b. redirect stdin to pipe read in
