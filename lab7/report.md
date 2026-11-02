# Lab 7 – Three Small Programs: Processes, Redirection, a Pipe (Linux)

Zastinceanu Mihai, FAF-243. The complete report with all figures is the .docx file.

## AIM OF THE WORK

### Purpose of the practical work

The purpose of the work is to answer three questions of the lecture by running small C programs on Linux: how a process is born, where printf writes, and how the command `ls | wc -l` works [1]. The programs were compiled with gcc 15.2.0 and the options -Wall -Wextra, without warnings, on the course virtual machine (Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic).

### Problem statement

The following tasks have to be solved:

- to run a program that does what the shell does for ls -l: fork, exec and wait
- to create a zombie process on purpose and to observe it in the process table
- to redirect the output of a child to a file with close and open
- to build the pipeline ls /etc | wc -l with pipe and dup, and to break it on purpose

## 1 FORK, EXEC AND WAIT

### 1.1 Theoretical background

A new process is created by fork, which makes a copy of the calling process; the copy receives the return value 0 and the original receives the PID of the copy [2]. The function execlp replaces the program of the process with another one, searched in PATH, and does not return when it succeeds [3]. The parent waits for the end of the child with waitpid, which also returns its exit status, extracted with WEXITSTATUS [4].

### 1.2 Work performed

The program run.c (Listing 1.1) was compiled and run (Figure 1.1): the parent printed its PID 39667, the child printed its PID 39668 and its parent 39667, became ls -l, and the parent reported the status 0.

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
*Listing 1.1 – The program run.c*

*Figure 1.1 – run.c compiled and run: the child becomes ls -l*

For the next two steps the line of execlp was replaced in two copies of the program, run_date.c and run_nosuch.c, so that run.c stays in its original form (Figure 1.2). With date the child printed the date and ended with the status 0. With nosuchprog the child printed "exec: No such file or directory" and the parent reported the status 127.

*Figure 1.2 – The child runs date, then a program that does not exist*

### 1.3 Answers to the questions

Question 1. A successful exec replaces the whole program of the child (code, data and stack) with ls or date, so the line perror("exec") no longer exists in that process and is never executed. Only when exec fails, as with nosuchprog, does it return, and the next line prints the error.

Question 2. The child prints as its parent the PID 39667, which is the PID printed by the parent itself, the process run that called fork. It is not the shell, which has the PID 28913.

Question 3. With ls (and with date) the parent sees the status 0, the exit code of the program. With nosuchprog it sees the status 127, the value of exit(127) after the failed exec, which is also the code that bash uses for a command that is not found.

## 2 A ZOMBIE ON PURPOSE

### 2.1 Theoretical background

When a process ends, the kernel keeps its entry in the process table, with the PID and the exit status, until the parent reads the status with wait. During this time the process is a zombie and has the state Z [4].

### 2.2 Work performed

The copy zombie.c differs from run.c only by the line sleep(20) before waitpid. The program was started in the background, and the process table was printed after 2 seconds and again after 20 seconds (Figure 2.1).

*Figure 2.1 – The zombie in ps, and the process table 20 seconds later*

### 2.3 Answers to the questions

Question 1. ls has the letter Z in the column STAT and is shown as [ls] <defunct>, with the PPID 39860 of the process zombie. Z means a zombie: the process has ended, but its entry remains until the parent waits for it.

Question 2. After 20 seconds the line [ls] <defunct> is gone. It disappeared because of the line `waitpid(pid, &status, 0);` (line 25 of zombie.c), executed when sleep(20) returned; then the process zombie itself ended.

## 3 REDIRECTION

### 3.1 Theoretical background

The functions open and dup always return the lowest free file descriptor [5]. The descriptors remain open across exec, so a program started after a redirection writes to the same descriptor number as usual, but the number now refers to another file. This is how the shell implements `ls -l > out.txt`.

### 3.2 Work performed

The program redirect.c (Listing 3.1) closes the descriptor 1 in the child, opens out.txt and runs ls -l (Figure 3.1).

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
*Listing 3.1 – The program redirect.c*

*Figure 3.1 – open returns the descriptor 1, and the listing goes to out.txt*

### 3.3 Answers to the questions

Question 1. open gave the descriptor 1, because it returns the lowest free number: 0 was still open and 1 had just been closed.

Question 2. ls writes to the descriptor 1 as always, and exec kept the open descriptors, so in ls the descriptor 1 is the file out.txt and not the terminal. The message "open gave descriptor 1" appeared on the screen because it was written to the descriptor 2. The listing even contains out.txt with the size 0, because the file was created before ls ran.

Question 3. To redirect the errors, the descriptor 2 must be closed before open; then open returns 2.

## 4 A PIPE

### 4.1 Theoretical background

The system call pipe creates a buffer in the kernel with a read end p[0] and a write end p[1] [6]. A reader of a pipe receives the end of file only when all write ends of the pipe are closed, in every process that holds them. Therefore every process must close the ends that it does not use.

### 4.2 Work performed

The program pipeline.c (Listing 4.1) creates one pipe and two children: in ls the descriptor 1 becomes p[1], and in wc the descriptor 0 becomes p[0]. Its result is compared with the shell in Figure 4.1; the optional version with three stages, pipeline3.c (Listing 4.2), was compared in the same way.

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
*Listing 4.1 – The program pipeline.c*

*Figure 4.1 – pipeline and pipeline3 compared with the shell*

Then the line `close(p[1]);` of the parent was turned into a comment in a copy, pipeline_broken.c. The program did not end and was stopped with Ctrl-C (Figure 4.2). It was started again in the background and inspected (Figure 4.3): ls had already ended, wc was sleeping in the kernel function anon_pipe_read, and the parent still held the write end of the same pipe as its descriptor 4. When the parent was killed, the last write end was closed, wc received the end of file and printed 244.

*Figure 4.2 – pipeline with close(p[1]) commented out never ends*

*Figure 4.3 – wc blocked in anon_pipe_read, released when the parent is killed*

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
*Listing 4.2 – The program pipeline3.c for ls /usr/bin | grep x | wc -l*

### 4.3 Answers to the questions

Question 1. ./pipeline and `ls /etc | wc -l` gave the same number, 244. The optional pipeline3 and the command `ls /usr/bin | grep x | wc -l` both gave 229.

Question 2. After the change wc never ends; ls ends normally and is no longer in the process table.

Question 3. wc waits for the end of file on the pipe, which comes only when all write ends are closed, but the parent still holds p[1] while it waits for wc, so the two processes wait for each other forever.

## CONCLUSIONS

Starting a program consists of two separate steps: fork copies the current process, and exec replaces the program of the copy. Because exec does not return when it succeeds, the code after it runs only on error, and the exit status travels back to the parent through wait. A child that has ended remains a zombie until its parent waits for it.

Redirection and pipes need no support from the programs: the descriptors are arranged between fork and exec using the rule that open and dup return the lowest free number, and ls or wc simply use the descriptors 0 and 1. Every unused end of a pipe must be closed, because the reader receives the end of file only when all write ends are closed; one forgotten close(p[1]) was enough to block the pipeline forever.

## REFERENCES

1. Three Small Programs: Processes, Redirection, a Pipe. Laboratory Work 7, Operating Systems course. FCIM, Technical University of Moldova, 2026.
2. fork(2). Linux manual page [online]. Available: https://man7.org/linux/man-pages/man2/fork.2.html
3. exec(3). Linux manual page [online]. Available: https://man7.org/linux/man-pages/man3/exec.3.html
4. wait(2). Linux manual page [online]. Available: https://man7.org/linux/man-pages/man2/wait.2.html
5. dup(2). Linux manual page [online]. Available: https://man7.org/linux/man-pages/man2/dup.2.html
6. pipe(7). Linux manual page [online]. Available: https://man7.org/linux/man-pages/man7/pipe.7.html
