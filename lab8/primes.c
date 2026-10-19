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
