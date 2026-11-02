/* count.c - print the same 1000 lines in two ways, to count the write calls.
 * Build:  gcc -Wall -Wextra -o count count.c
 * Run:    strace -c -e trace=write ./count raw  > out.txt
 *         strace -c -e trace=write ./count buf  > out.txt
 */
#include <stdio.h>      /* printf, snprintf */
#include <string.h>     /* strcmp, strlen */
#include <unistd.h>     /* write */

int main(int argc, char *argv[]) {
    int raw = argc > 1 && strcmp(argv[1], "raw") == 0;
    char line[32];

    for (int i = 1; i <= 1000; i++) {
        if (raw) {
            /* one system call for every line */
            int len = snprintf(line, sizeof(line), "line %d\n", i);
            write(1, line, len);
        } else {
            /* printf only fills a buffer in the C library */
            printf("line %d\n", i);
        }
    }
    return 0;
}
