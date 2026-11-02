#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

// sysinfo: print the free memory and the number of processes in use
int
main(int argc, char *argv[])
{
  struct sysinfo info;

  if (sysinfo(&info) < 0) {
    fprintf(2, "sysinfo: failed\n");
    exit(1);
  }
  printf("free memory: %lu bytes (%lu pages)\n", info.freemem, info.freemem / 4096);
  printf("processes in use: %lu\n", info.nproc);
  exit(0);
}
