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

The xv6 programs go into `~/xv6-riscv/user/`; each Makefile contains the UPROGS lines up to that lab.
Lab 3: `bash oneliners.sh` and `./report.sh access.log` run inside `~/os-lab3`.
Lab 9: `gcc -Wall -Wextra -o syscalls syscalls.c` and `strace -e trace=openat,write,read,close ./syscalls`.
Lab 10: the kernel changes are in `trace.patch`; apply them in `~/xv6-riscv` with `git am ~/os-2026-ZastinceanuMihai/lab10/trace.patch`.
