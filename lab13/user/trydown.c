// trydown.c: shows both answers of sem_trydown (Lab 13, optional).
// sem_trydown takes one unit if there is one (returns 0),
// otherwise it returns -1 at once instead of sleeping like sem_down.
#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  sem_init(4, 1);                                   // semaphore 4 starts at 1
  printf("value 1: sem_trydown(4) = %d\n", sem_trydown(4));   // takes it: 0
  printf("value 0: sem_trydown(4) = %d\n", sem_trydown(4));   // nothing left: -1, no sleep
  sem_up(4);                                        // give one back
  printf("after sem_up: sem_trydown(4) = %d\n", sem_trydown(4)); // 0 again
  printf("bad id 9: sem_trydown(9) = %d\n", sem_trydown(9));    // -1
  exit(0);
}
