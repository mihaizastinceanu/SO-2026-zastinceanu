# Labs 7 and 8 – Processes, Redirection, a Pipe (Linux) and the Same Calls in xv6

Zastinceanu Mihai, FAF-243. The full report with all screenshots is the .docx file in this folder.

## Purpose of the work

Lab 7: answer three questions from the lecture by running small C programs on Linux: how is a process born (fork + exec + wait), what is a zombie, where does printf write (descriptors and redirection), and how does `ls | wc -l` work (pipe + dup).

Lab 8: run the same fork/exec/wait program inside xv6, the OS built during the semester, to see that the calls are the same, and make two processes talk through pipes (pingpong); optionally, the concurrent prime sieve (primes).

Environment: the course VirtualBox VM, Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic, gcc 15.2.0 (all programs compiled with `-Wall -Wextra` without warnings); xv6-riscv commit `06aad25`, riscv64-linux-gnu-gcc 15.2.0 with `-Werror`, QEMU 10.2.1. Figures are screenshots of the VM terminal, numbered lab.part.n.

## Lab 7. Three Small Programs (Linux)

### Part 1. fork + exec + wait

```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* the three steps of the shell for the command:   ls -l   */
int main(void)
{
    printf("parent: my PID is %d\n", getpid());
    fflush(stdout);

    pid_t pid = fork();                          /* step 1: copy myself */
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {                              /* only the child runs this */
        printf("child:  my PID is %d, my parent is %d\n", getpid(), getppid());
        fflush(stdout);
        execlp("ls", "ls", "-l", (char *)NULL);  /* step 2: become ls */
        perror("exec");                          /* runs only if exec failed */
        exit(127);
    }

    int status;
    waitpid(pid, &status, 0);                    /* step 3: wait for the child */
    printf("parent: child %d ended with status %d\n", pid, WEXITSTATUS(status));
    return 0;
}
```

*Figure 7.1.1 – run.c compiled and run: the child becomes ls -l (screenshot in the .docx report)*

For steps 1.3 and 1.4 the execlp line was replaced (copies `run_date.c` and `run_nosuch.c`, so that the submitted `run.c` stays the original):

*Figure 7.1.2 – The child runs date, then a program that does not exist (screenshot in the .docx report)*

**1. The line perror("exec") is in the child, right after execlp. Why is it printed only with nosuchprog?**

A successful exec replaces the whole program of the child (code, data, stack) with `ls` or `date`; the code of run, including the perror line, no longer exists in that process, so exec never returns. Only when exec fails (no file `nosuchprog` in PATH, ENOENT) does it return -1, and then the next line runs: `exec: No such file or directory`.

**2. Which number does the child print as "my parent", and whose PID is it?**

In Figure 7.1.1 the child prints `my parent is 39667`, which is the PID the parent printed (`parent: my PID is 39667`): the `run` process that called fork. It is not the shell (the shell is 28913).

**3. What status does the parent see with ls, and what status with nosuchprog?**

With `ls` (and `date`) status 0, the exit code of the program. With `nosuchprog` status 127: the child's own `exit(127)` after the failed exec, the same code bash uses for "command not found". `WEXITSTATUS(status)` extracts it from the status word.

### Part 2. A zombie, on purpose

```
    int status;
    sleep(20);                                   /* the new line */
    waitpid(pid, &status, 0);                    /* step 3: wait for the child */
```

*Figure 7.2.1 – The zombie in ps, and the process table 20 seconds later (screenshot in the .docx report)*

**1. Which letter does ls have in the STAT column, and what does it mean?**

`Z` – zombie, shown as `[ls] <defunct>` with PPID 39860 (./zombie). ls has finished, but the kernel keeps its process-table entry (PID and exit status) until the parent collects it with wait.

**2. After 20 seconds, is the [ls] <defunct> line still there? Which line of zombie.c made it disappear?**

No. `waitpid(pid, &status, 0);` (line 25 of zombie.c), executed after `sleep(20)` returned, read the status and let the kernel free the entry; then zombie itself ended (`[1]+ Done`).

### Part 3. Redirection: ls -l > out.txt

```
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

/* does the same as the shell command:   ls -l > out.txt   */
int main(void)
{
    pid_t pid = fork();
    if (pid == 0) {
        close(1);                                /* descriptor 1 is now free */
        int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) { perror("open"); exit(1); }
        fprintf(stderr, "open gave descriptor %d\n", fd);
        execlp("ls", "ls", "-l", (char *)NULL);  /* ls writes to 1, as always */
        perror("exec");
        exit(127);
    }
    waitpid(pid, NULL, 0);
    printf("done: the listing is in out.txt\n");
    return 0;
}
```

*Figure 7.3.1 – redirect: open returns descriptor 1 and the listing goes to out.txt (screenshot in the .docx report)*

**1. Which number did open give, and why exactly that one?**

1. open always returns the lowest free descriptor number. 0 (stdin) was still open and 1 had just been closed, so 1 was the lowest free one.

**2. ls was not changed at all. Why does its listing not appear on the screen?**

ls writes to descriptor 1, as always. exec keeps the open descriptors, so in the new program 1 is the file out.txt, not the terminal. The message "open gave descriptor 1" still appeared because it was written to stderr (2). The listing even contains `out.txt` with size 0: the file was created and truncated before ls ran.

**3. To redirect the errors instead (like 2> err.txt), which descriptor would you close before open?**

Descriptor 2: `close(2)`, then `open("err.txt", ...)` returns 2, and everything the program writes to stderr goes to the file.

### Part 4. A pipe: ls /etc | wc -l

```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* does the same as the shell command:   ls /etc | wc -l   */
int main(void)
{
    int p[2];
    if (pipe(p) < 0) { perror("pipe"); return 1; }

    if (fork() == 0) {                           /* ls: its 1 becomes p[1] */
        close(1);
        dup(p[1]);                               /* smallest free number: 1 */
        close(p[0]);
        close(p[1]);
        execlp("ls", "ls", "/etc", (char *)NULL);
        perror("exec ls");
        exit(127);
    }

    if (fork() == 0) {                           /* wc: its 0 becomes p[0] */
        close(0);
        dup(p[0]);                               /* smallest free number: 0 */
        close(p[0]);
        close(p[1]);
        execlp("wc", "wc", "-l", (char *)NULL);
        perror("exec wc");
        exit(127);
    }

    close(p[0]);                                 /* the parent uses neither end */
    close(p[1]);
    wait(NULL);
    wait(NULL);
    return 0;
}
```

*Figure 7.4.1 – pipeline and pipeline3 compared with the real shell (screenshot in the .docx report)*

Then the line `close(p[1]);` in the parent was commented out (copy `pipeline_broken.c`). The program no longer ended and was stopped with Ctrl-C (Figure 7.4.2). To see why, it was started again in the background and inspected (Figure 7.4.3): ls had already exited, wc was sleeping in `anon_pipe_read`, and the parent still had the write end of the same pipe open as its descriptor 4. As soon as the parent was killed, that last write end closed, wc received end-of-file and printed 244.

*Figure 7.4.2 – pipeline with close(p[1]) commented out never ends (stopped with Ctrl-C) (screenshot in the .docx report)*

*Figure 7.4.3 – wc blocked in anon_pipe_read; killing the parent releases it (screenshot in the .docx report)*

**1. Did ./pipeline and ls /etc | wc -l give the same number?**

Yes: 244 and 244. The optional three-stage version also matched: `./pipeline3` and `ls /usr/bin | grep x | wc -l` both printed 229.

**2. After the change, which of the two children never ends, ls or wc?**

wc. ls wrote its list and exited normally (it is not in the ps output); wc stays in state S.

**3. Why, in one sentence: what is wc waiting for?**

wc reads until end-of-file, and a pipe gives end-of-file only when every write end is closed, but the parent still holds p[1] while it waits for wc, so both wait for each other forever.

#### Optional: pipeline3.c (ls /usr/bin | grep x | wc -l)

```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* does the same as the shell command:   ls /usr/bin | grep x | wc -l   */

int a[2], b[2];                                  /* a: ls -> grep, b: grep -> wc */

/* every process closes all four pipe ends once it has what it needs */
static void close_all(void)
{
    close(a[0]); close(a[1]);
    close(b[0]); close(b[1]);
}

int main(void)
{
    if (pipe(a) < 0 || pipe(b) < 0) { perror("pipe"); return 1; }

    if (fork() == 0) {                           /* ls: its 1 becomes a[1] */
        close(1);
        dup(a[1]);
        close_all();
        execlp("ls", "ls", "/usr/bin", (char *)NULL);
        perror("exec ls");
        exit(127);
    }

    if (fork() == 0) {                           /* grep: 0 is a[0], 1 is b[1] */
        close(0);
        dup(a[0]);
        close(1);
        dup(b[1]);
        close_all();
        execlp("grep", "grep", "x", (char *)NULL);
        perror("exec grep");
        exit(127);
    }

    if (fork() == 0) {                           /* wc: its 0 becomes b[0] */
        close(0);
        dup(b[0]);
        close_all();
        execlp("wc", "wc", "-l", (char *)NULL);
        perror("exec wc");
        exit(127);
    }

    close_all();                                 /* the parent uses none of them */
    wait(NULL);
    wait(NULL);
    wait(NULL);
    return 0;
}
```

## Lab 8. The Same Calls, Inside Your Own OS (xv6)

New programs: `user/run.c`, `user/pingpong.c`, `user/primes.c`, each with a line in UPROGS (a TAB at the start, nothing after the backslash). `zombie` is already one of the standard xv6 programs.

*Figure 8.0.1 – The UPROGS lines and the system calls declared in user/user.h (screenshot in the .docx report)*

### Part 1. run, inside xv6

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

*Figure 8.1.1 – run in xv6: the child becomes echo (screenshot in the .docx report)*

*Figure 8.1.2 – The child runs ls (args = { "ls", 0 }) (screenshot in the .docx report)*

*Figure 8.1.3 – exec("nosuchprog", args): exec failed, status 1 (screenshot in the .docx report)*

For 1.3 and 1.4 the two lines were changed, rebuilt and run, then `run.c` was restored to the echo version that is submitted.

**1. Write the three calls that run.c uses in xv6 and the three calls run.c used on Linux in Lab 7. What is the same?**

xv6: `fork()`, `exec("echo", args)`, `wait(&status)`. Linux: `fork()`, `execlp("ls", "ls", "-l", NULL)`, `waitpid(pid, &status, 0)`. The idea and the order are the same: copy the process, replace the copy's program, wait for it; `fork` is identical. The differences are only in the library: on Linux `execlp` is a glibc wrapper over the `execve` system call that takes a variable list and searches PATH, and `waitpid` can wait for one specific child, while xv6 `exec` takes an argv array and `wait` waits for any child.

**2. Who prints "hello from the child": the code of run, or echo?**

echo. The child printed `child: my pid is 4` with run's code, then `exec` replaced its program with `/echo`, and echo printed its arguments `hello from the child`, still as process 4.

**3. Why is the status 1 here and 127 on Linux? (Look at the exit line after exec in both programs.)**

The status is whatever the child passes to exit when exec fails: the xv6 run.c has `exit(1)` after exec, the Linux run.c has `exit(127)`. (xv6 `wait` stores the exit value directly; on Linux the value is packed in the status word and read with `WEXITSTATUS`.)

### Part 2. pingpong

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

*Figure 8.2.1 – pingpong (twice) and the optional primes (screenshot in the .docx report)*

The child (pid 4) prints "received ping" first, then the parent (pid 3) prints "received pong"; the second run gives 6 and 5.

**1. Why do you need TWO pipes, and not one for both directions?**

A pipe is a one-way channel with one buffer: whoever reads first gets the byte. With a single pipe the parent could read back its own ping before the child gets it (or the child its own pong), and there would be no way to tell whose byte it is. One pipe per direction (p2c and c2p) makes every byte go to the right process; it also lets each side close the ends it does not use.

**2. Nothing in your code says "wait until the child has replied". Which call makes the parent wait?**

`read(c2p[0], &b, 1)`. Reading from an empty pipe blocks: the kernel puts the parent to sleep in piperead until the child writes the byte (or every write end is closed). In the same way the child's `read(p2c[0], ...)` blocks until the ping arrives, which is why "ping" is always printed first.

#### Optional: primes

Each stage reads its first number p from the left pipe (it is always a prime), prints it, creates a pipe and a child for the next stage, and forwards only the numbers not divisible by p. Every unused pipe end is closed, otherwise the stages would never see end-of-file and xv6 would run out of descriptors. Output: the 11 primes from 2 to 31 (Figure 8.2.1).

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

## Conclusions

Starting a program is two separate steps: fork copies the current process, and exec replaces the copy's program. Because exec does not return on success, the code after it runs only on error, and the exit status (0, 127, or 1 in xv6) travels back to the parent through wait. A finished child stays a zombie until its parent waits for it.

Redirection and pipes need no help from the programs: the shell (here our own code) arranges the descriptors between fork and exec using the rule "open/dup returns the lowest free number", and ls or wc simply use 0 and 1. The one rule to remember is to close every unused pipe end: a reader gets end-of-file only when all write ends are closed, and one forgotten `close(p[1])` was enough to block the pipeline forever.

The same program ran almost unchanged inside xv6: `fork`, `exec`, `wait`, `pipe`, `read`, `write` and `close` are the same system calls in our small kernel. pingpong showed that a blocking `read` is a synchronisation tool, and primes showed a chain of processes connected by pipes.
