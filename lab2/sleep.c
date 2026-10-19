#include "kernel/types.h"
#include "user/user.h"

// sleep <ticks>: pause for the given number of clock ticks.
//
// Note: the lab sheet calls sleep(), but in current xv6 the user-level
// system call sleep() was renamed to pause() (commit f5dea58, Aug 2025,
// "user sleep() -> pause()"), because the kernel uses the name sleep()
// for its own internal sleep/wakeup mechanism. pause() is the same
// system call (SYS_pause, handled by sys_pause() in kernel/sysproc.c).
int
main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(2, "usage: sleep <ticks>\n");
    exit(1);
  }
  pause(atoi(argv[1])); // system call into the kernel
  exit(0);
}
