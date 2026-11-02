# Lab 8 – The Same Calls, Inside Your Own OS (xv6)

Zastinceanu Mihai, FAF-243. The complete report with all figures is the .docx file.

## AIM OF THE WORK

### Purpose of the practical work

The purpose of the work is to run the fork, exec and wait program of the seventh laboratory work inside xv6, the operating system built during the semester, and to see that the system calls are the same; then to make two processes communicate through pipes [1]. The work was done with xv6-riscv commit 06aad25, the cross compiler riscv64-linux-gnu-gcc 15.2.0 (with -Werror) and QEMU 10.2.1.

### Problem statement

The following tasks have to be solved:

- to write user/run.c, in which a child becomes echo, and to run it with ls and with a missing program
- to write user/pingpong.c, in which a parent and a child exchange one byte through two pipes
- to write the optional program user/primes.c, which prints the primes up to 35 with a chain of processes

## 1 THE PROGRAM RUN INSIDE XV6

### 1.1 Theoretical background

xv6 has no C library: its user programs call the system calls of the kernel directly, through the stubs generated from user/usys.pl [2]. The calls fork, exec and wait have the same meaning as on Linux, but exec takes an array of arguments and does not search a PATH, and wait waits for any child and stores its exit value directly [3].

### 1.2 Work performed

The programs of the work were added to UPROGS, and the declarations of the system calls were read in user/user.h (Figure 1.1).

*Figure 1.1 – The UPROGS lines and the system calls declared in user/user.h*

The program user/run.c (Listing 1.1) was run in xv6 (Figure 1.2): the parent printed its pid 3, the child printed its pid 4 and became echo, which printed "hello from the child", and the parent reported the status 0.

```
#include "kernel/types.h"
#include "user/user.h"

// the three steps of the shell, inside xv6:  echo hello from the child
int
main(int argc, char *argv[])
{
  printf("parent: my pid is %d\n", getpid());

  int pid = fork();                     // step 1: copy myself
  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  if(pid == 0){                         // only the child runs this
    printf("child: my pid is %d\n", getpid());
    char *args[] = { "echo", "hello", "from", "the", "child", 0 };
    exec("echo", args);                 // step 2: become echo
    printf("exec failed\n");            // runs only if exec failed
    exit(1);
  }

  int status;
  wait(&status);                        // step 3: wait for the child
  printf("parent: child %d ended with status %d\n", pid, status);
  exit(0);
}
```
*Listing 1.1 – The program user/run.c*

*Figure 1.2 – run in xv6: the child becomes echo*

Then the two lines of exec were changed to run ls, and later a program that does not exist; after each change xv6 was rebuilt. With ls the child listed the root directory (Figure 1.3); with nosuchprog it printed "exec failed", and the parent reported the status 1 (Figure 1.4). Finally run.c was restored to the version with echo.

*Figure 1.3 – The child runs ls*

*Figure 1.4 – exec of a missing program: exec failed and the status 1*

### 1.3 Answers to the questions

Question 1. In xv6, run.c uses fork(), exec("echo", args) and wait(&status). On Linux, run.c used fork(), execlp("ls", "ls", "-l", NULL) and waitpid(pid, &status, 0). The idea and the order are the same: copy the process, replace the program of the copy, wait for it; fork is identical. The differences are in the library: execlp is a wrapper over the system call execve that takes a list of arguments and searches PATH, and waitpid can wait for a specific child.

Question 2. "hello from the child" is printed by echo. The child printed "child: my pid is 4" with the code of run, then exec replaced its program with echo, which printed its arguments as the same process 4.

Question 3. The status is the value that the child passes to exit after a failed exec: the xv6 run.c calls exit(1) and the Linux run.c calls exit(127). xv6 wait stores this value directly, while on Linux it is packed in the status word and read with WEXITSTATUS.

## 2 PINGPONG

### 2.1 Theoretical background

A pipe is a one-way channel with a single buffer in the kernel. A read from an empty pipe blocks the reading process until data is written or until all write ends are closed, so a blocking read can be used to synchronise two processes [2], [4].

### 2.2 Work performed

The program user/pingpong.c (Listing 2.1) was written from the template of the handout: the parent writes one byte into the pipe p2c, the child reads it, prints "received ping" and writes the byte back into c2p, and the parent reads it and prints "received pong". The program was run twice (Figure 2.1); the child (pid 4, then 6) always printed first.

```
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p2c[2], c2p[2];     // parent-to-child, child-to-parent
  char b = 'x';

  pipe(p2c);
  pipe(c2p);

  if(fork() == 0){
    close(p2c[1]); close(c2p[0]);       // the ends the child does not use
    read(p2c[0], &b, 1);                // wait for the ping
    printf("%d: received ping\n", getpid());
    write(c2p[1], &b, 1);               // send the byte back
    exit(0);
  }

  close(p2c[0]); close(c2p[1]);         // the ends the parent does not use
  write(p2c[1], &b, 1);                 // ping
  read(c2p[0], &b, 1);                  // blocks until the child answers
  printf("%d: received pong\n", getpid());
  wait(0);
  exit(0);
}
```
*Listing 2.1 – The program user/pingpong.c*

The optional program user/primes.c (Listing 2.2) builds a chain of processes: each stage reads its first number p, which is a prime, prints it, creates a pipe and a child for the next stage and forwards only the numbers that p does not divide. Every unused end of a pipe is closed, otherwise the stages would never receive the end of file and xv6 would run out of descriptors. The output contains the 11 primes from 2 to 31 (Figure 2.1).

```
#include "kernel/types.h"
#include "user/user.h"

// primes: print the primes from 2 to 35 with a chain of processes.
// Each stage reads its first number p (a prime), prints it, and passes
// on to the next stage only the numbers that p does not divide.

void
stage(int left) __attribute__((noreturn));

void
stage(int left)
{
  int p, n;
  int right[2];

  if(read(left, &p, sizeof(p)) != sizeof(p)){
    close(left);                        // nothing left: the chain ends here
    exit(0);
  }
  printf("prime %d\n", p);

  pipe(right);
  if(fork() == 0){
    close(right[1]);
    close(left);                        // the next stage reads only from right
    stage(right[0]);
  }
  close(right[0]);
  while(read(left, &n, sizeof(n)) == sizeof(n)){
    if(n % p != 0)
      write(right[1], &n, sizeof(n));
  }
  close(left);
  close(right[1]);                      // EOF for the next stage
  wait(0);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int first[2];

  pipe(first);
  if(fork() == 0){
    close(first[1]);
    stage(first[0]);
  }
  close(first[0]);
  for(int i = 2; i <= 35; i++)
    write(first[1], &i, sizeof(i));
  close(first[1]);
  wait(0);
  exit(0);
}
```
*Listing 2.2 – The program user/primes.c*

*Figure 2.1 – pingpong run twice, and the optional program primes*

### 2.3 Answers to the questions

Question 1. Two pipes are needed because a pipe is one-way and has a single buffer: whoever reads first takes the byte. With one pipe the parent could read back its own ping before the child, or the child its own pong, and nothing would show for whom a byte is meant. With one pipe for each direction every byte reaches the right process.

Question 2. The parent waits in the call read(c2p[0], &b, 1): a read from an empty pipe blocks, and the kernel puts the parent to sleep until the child writes the byte. In the same way the read of the child waits for the ping, which is why "received ping" is always printed first.

## CONCLUSIONS

The program that started a child with fork, exec and wait ran almost unchanged in xv6, because xv6 offers the same system calls as Linux; the differences are only in the library layer, such as the search in PATH done by execlp and the WEXITSTATUS macro. The exit status after a failed exec is simply the value that the program itself passes to exit.

The program pingpong showed that a pipe is a one-way channel, so two-way communication needs two pipes, and that a blocking read is enough to order the actions of two processes without any other synchronisation. The program primes extended this to a chain of processes connected by pipes, in which closing every unused end is essential.

## REFERENCES

1. The Same Calls, Inside Your Own OS. Laboratory Work 8, Operating Systems course. FCIM, Technical University of Moldova, 2026.
2. COX, R., KAASHOEK, F., MORRIS, R. xv6: a simple, Unix-like teaching operating system [online]. MIT, 2024. Available: https://pdos.csail.mit.edu/6.1810/2024/xv6/book-riscv-rev4.pdf
3. xv6 for RISC-V. Source code repository, MIT PDOS [online]. Available: https://github.com/mit-pdos/xv6-riscv
4. Lab: Xv6 and Unix utilities. MIT 6.1810 Operating System Engineering [online]. Available: https://pdos.csail.mit.edu/6.1810/2024/labs/util.html
