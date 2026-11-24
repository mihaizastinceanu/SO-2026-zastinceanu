# Operating Systems 2026-2027 – laboratory works

Zastinceanu Mihai, FAF-243 (FCIM / FAF, Technical University of Moldova)

| Folder | Lab | Contents |
|---|---|---|
| lab1/ | 1. The OS as a Resource Manager (Linux) | Lab1-2_ZastinceanuMihai_FAF-243.docx (report for labs 1 and 2), report.md |
| lab2/ | 2. Meet the OS You Will Build (xv6) | sleep.c, pingpong.c, notes.md |
| lab3/ | 3. The Command Line: Shell, Pipes and Text Filters | Lab3_ZastinceanuMihai_FAF-243.docx, oneliners.sh, report.sh |
| lab4/ | 4. The xv6 Shell from the Outside | Lab4_ZastinceanuMihai_FAF-243.docx, sleep.c, args.c, args_linux.c, Makefile |
| lab5/ | 5. The File System with Your Own Hands (Linux) | Lab5_ZastinceanuMihai_FAF-243.docx, report.md |
| lab6/ | 6. Inside the xv6 File System | Lab6_ZastinceanuMihai_FAF-243.docx, mystat.c, find.c, Makefile, notes.md |
| lab7/ | 7. Three Small Programs (Linux) | Lab7_ZastinceanuMihai_FAF-243.docx, run.c, zombie.c, redirect.c, pipeline.c, pipeline3.c, report.md |
| lab8/ | 8. The Same Calls Inside Your Own OS (xv6) | Lab8_ZastinceanuMihai_FAF-243.docx, run.c, pingpong.c, primes.c, Makefile, notes.md |
| lab9/ | 9. Watching System Calls with strace (Linux) | Lab9_ZastinceanuMihai_FAF-243.docx, syscalls.c, count.c, report.md, img/ |
| lab10/ | 10. Add Your Own System Call: trace (xv6) | Lab10_ZastinceanuMihai_FAF-243.docx, kernel/ and user/ (the modified xv6 files), trace.patch, Makefile, notes.md, img/ |
| lab11/ | 11. The Race and the Semaphore (Linux) | Lab11_ZastinceanuMihai_FAF-243.docx, race.c, mutex.c, sem.c, order.c, parking.c, atomic.c (optional), report.md, img/ |
| lab12/ | 12. Producer, Consumer and the Other Tools (Linux) | Lab12_ZastinceanuMihai_FAF-243.docx, prodcons.c, condvar.c, readers.c, deadlock.c (the fixed version), report.md, img/ |
| lab13/ | 13. Semaphores in Your Own Kernel (xv6) | Lab13_ZastinceanuMihai_FAF-243.docx, kernel/ and user/ (sem.c, semtest.c, mix.c, trydown.c and the changed xv6 files), semaphores.patch, Makefile, notes.txt, notes.md, img/ |
| lab17/ | 17. Threads Share Memory, and Race (Linux) | Lab17_ZastinceanuMihai_FAF-243.docx, fire.c, fire4.c (Part 2, four threads), report.md, img/ |
| lab18/ | 18. Build a User-Level Thread Switch (xv6) | Lab18_ZastinceanuMihai_FAF-243.docx, user/ (uthread.c, uthread_switch.S), uthread.patch, Makefile, notes.txt, notes.md, img/ |

The xv6 programs go into `~/xv6-riscv/user/`; each Makefile contains the UPROGS lines up to that lab.
Lab 3: `bash oneliners.sh` and `./report.sh access.log` run inside `~/os-lab3`.
Lab 9: `gcc -Wall -Wextra -o syscalls syscalls.c` and `strace -e trace=openat,write,read,close ./syscalls`.
Lab 10: the kernel changes are in `trace.patch`; apply them in `~/xv6-riscv` with `git am ~/os-2026-ZastinceanuMihai/lab10/trace.patch`.
Lab 11: inside `~/os-lab11`, `gcc -Wall -pthread -o race race.c` (no -O2), the same for mutex, sem, order, parking and atomic.
Lab 12: inside `~/os-lab12`, `gcc -Wall -pthread -o prodcons prodcons.c` (and condvar, readers, deadlock); `timeout 5 ./deadlock; echo "exit code: $?"`.
Lab 13: apply the four commits in `~/xv6-riscv` with `git am ~/os-2026-ZastinceanuMihai/lab13/semaphores.patch`, then `make qemu` and run `semtest`, `mix`, `mix lock`, `trydown`. sem_init, sem_down, sem_up = 24, 25, 26, sem_trydown = 27 (sysinfo of Lab 10 moved to 28).
Lab 17: inside `~/os-lab17`, `gcc -Wall -Wextra -pthread -o fire fire.c` (and fire4.c), run `./fire` and `./fire4` several times.
Lab 18: apply the commit in `~/xv6-riscv` with `git am ~/os-2026-ZastinceanuMihai/lab18/uthread.patch`, then `make qemu` and run `uthread`.
