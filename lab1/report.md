# Lab 1 – The OS as a Resource Manager (Linux)

Student VM: `zastinceanu-mihai-VBox` · Ubuntu, kernel `7.0.0-34-generic` · date 2026-09-28

All outputs below were copied from the real terminal on this VM. The raw outputs are also saved in `~/os-lab1/out/`.

## Before you start

```
$ whoami
papa_bear
$ uname -a
Linux zastinceanu-mihai-VBox 7.0.0-34-generic #34-Ubuntu SMP PREEMPT_DYNAMIC Wed Sep  2 14:29:37 UTC 2026 x86_64 GNU/Linux
$ uptime
 21:28:08 up 22 min,  1 user,  load average: 0.98, 0.98, 0.70
```

So I am the user `papa_bear`, the kernel managing the machine is Linux 7.0.0 (x86_64), and the OS had been running for 22 minutes.

---

## Part 1 – Files and directories (file management)

Commands I ran: `pwd`, `ls -la /`, `ls -la ~`, `cd /etc && ls | head`, then in `~/os-lab1/demo`: `echo … > note.txt`, `cp`, `mv`, `ls -l`, `rm`, `chmod 600`, `chmod 644`.

Interesting output (the tree changing):

```
$ ls -l                     # after cp + mv
-rw-rw-r-- 1 papa_bear papa_bear 24 Sep 28 21:28 note.txt
-rw-rw-r-- 1 papa_bear papa_bear 24 Sep 28 21:28 renamed.txt
$ rm renamed.txt
$ chmod 600 note.txt
$ ls -l note.txt
-rw------- 1 papa_bear papa_bear 24 Sep 28 21:28 note.txt
$ chmod 644 note.txt
$ ls -l note.txt
-rw-r--r-- 1 papa_bear papa_bear 24 Sep 28 21:28 note.txt
```

**Observe (1) – Who owns the files, and which group?**
The owner is my user `papa_bear` and the group is also `papa_bear` (Ubuntu gives every user their own group with the same name, `id` shows `gid=1000(papa_bear)`). The OS automatically marks new files with whoever created them.

**Observe (2) – What do the ten characters mean after `chmod 600`?**
`-rw-------`
- 1st char `-` = it is a normal file (a `d` would mean directory, `l` a link).
- chars 2–4 `rw-` = the **owner** can read and write, not execute.
- chars 5–7 `---` = the **group** can do nothing.
- chars 8–10 `---` = **others** (everyone else) can do nothing.
So after `chmod 600` only I can read/write the file. (6 = 4 read + 2 write, 0 = nothing.)

**Observe (3) – Two directories directly under `/`:**
- `/etc` – system configuration files (I saw `NetworkManager`, `adduser.conf`, `X11` … in it).
- `/home` – the home folders of normal users (mine is `/home/papa_bear`).
- (also: `/dev` – device files, `/proc` – live info about processes made up by the kernel, size 0 on disk.)

---

## Part 2 – Processes (process management)

```
$ ps aux | head -2
USER         PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
root           1  0.1  0.3  25836 13536 ?        Ss   21:05   0:02 /usr/lib/systemd/systemd --switched-root --system --deserialize=52 splash
$ ps aux | wc -l
262
```

`top` is interactive, so I also took one snapshot with `top -b -n 1`:

```
top - 21:28:22 up 22 min,  1 user,  load average: 0.83, 0.95, 0.69
Tasks: 261 total,   2 running, 258 sleeping,   0 stopped,   1 zombie
%Cpu(s): 17.4 us,  6.5 sy,  0.0 ni, 73.9 id,  0.0 wa,  0.0 hi,  2.2 si,  0.0 st
```

Background process start / find / kill:

```
$ sleep 300 &
PID printed: 8378
$ jobs
[1]+  8378 Running                    sleep 300 &
$ ps -ef | grep sleep          (long shell lines removed)
papa_be+    8378    8376  0 21:28 ?        00:00:00 sleep 300
papa_be+    8380    8376  0 21:28 ?        00:00:00 grep sleep
$ kill 8378
[1]+  8378 Terminated                 sleep 300
```

`/proc` for a second sleep (PID 8383): `ls /proc/8383/` showed ~58 entries such as `cmdline`, `cwd`, `exe`, `fd`, `maps`, `status`.

```
$ cat /proc/$PID/status | head -20
Name:	sleep
Umask:	0002
State:	S (sleeping)
Tgid:	8383
Pid:	8383
PPid:	8376
Uid:	1000	1000	1000	1000
...
VmPeak:	   16116 kB
VmSize:	   16116 kB
```

**Observe (1) – PID 1?**
PID 1 is `systemd` (`/usr/lib/systemd/systemd`, run by `root`). It is the **init** process: the first user-space program the kernel starts at boot. It starts all the other services, and it adopts orphaned processes.

**Observe (2) – How many processes?**
`ps aux | wc -l` gave 262 lines, one is the header, so about **261 processes** (top agreed: `Tasks: 261 total`). Most of them are sleeping (258). Many are kernel threads in `[brackets]` like `[kthreadd]` and `[kworker/...]`.

**Observe (3) – The "State:" line?**
`State:	S (sleeping)`. The `sleep` process is not using the CPU. It is waiting for its timer, so the scheduler keeps it off the CPU until then.

---

## Part 3 – Memory (memory management)

```
$ free -h
               total        used        free      shared  buff/cache   available
Mem:           3.3Gi       1.8Gi       159Mi        49Mi       1.5Gi       1.5Gi
Swap:          3.8Gi       654Mi       3.2Gi
$ cat /proc/meminfo | head -6
MemTotal:        3479664 kB
MemFree:          162612 kB
MemAvailable:    1591704 kB
Buffers:           14748 kB
Cached:          1452496 kB
SwapCached:        42652 kB
$ grep VmRSS /proc/$PID/status
VmRSS:	    7868 kB
```

Extra lines I looked at: `RssAnon: 1228 kB`, `RssFile: 6640 kB`, and `swapon --show` → `/swap.img file 3.8G 661.7M`.

**Observe (1) – Total and free RAM?**
Total: **3.3 GiB** (3,479,664 kB). Truly "free": only **159 MiB**, but "available" is **1.5 GiB**, because Linux uses spare RAM as a cache (1.5 GiB buff/cache) and can give it back when a program needs it.

**Observe (2) – What is swap and how much?**
Swap is space on the disk that the OS uses as extra "slow RAM": when RAM gets full, it moves pages that are not being used to the disk. Here it is a **3.8 GiB** swap file `/swap.img` (I also saw it in `ls -la /`), and about 654 MiB of it was already in use (Firefox and the desktop use a lot of RAM on this 3.3 GiB VM).

**Observe (3) – VmRSS of a bare `sleep`?**
**7868 kB (~7.7 MB)**. At first that surprised me because `sleep` does nothing. But most of it (`RssFile` 6640 kB) is shared library code like libc that is mapped into the process. Only ~1.2 MB (`RssAnon`) is private to this process. The shared pages are counted in every process's RSS, but they are really in RAM only once.

---

## Part 4 – Devices and storage (I/O management)

```
$ df -h | grep ' /$'
/dev/sda2        49G   13G   35G  27% /
$ lsblk | tail -4
sda      8:0    0    50G  0 disk
├─sda1   8:1    0     1M  0 part
└─sda2   8:2    0    50G  0 part /
sr0     11:0    1  1024M  1 rom
$ du -sh ~/os-lab1
44K	/home/papa_bear/os-lab1
$ mount | head -2
tmpfs on /run type tmpfs (rw,nosuid,nodev,size=695936k,nr_inodes=819200,mode=755,inode64)
/dev/sda2 on / type ext4 (rw,relatime)
$ ls -l /dev/sda /dev/null /dev/tty /dev/random
crw-rw-rw- 1 root root 1, 3 Sep 28 21:05 /dev/null
crw-rw-rw- 1 root root 1, 8 Sep 28 21:05 /dev/random
brw-rw---- 1 root disk 8, 0 Sep 28 21:05 /dev/sda
crw-rw-rw- 1 root tty  5, 0 Sep 28 21:05 /dev/tty
```

`lsblk` also listed 20 `loop` devices, all Ubuntu snap packages mounted read-only under `/snap/...`.

**Observe (1) – Which device is `/` on?**
`/dev/sda2`, the second partition of the 50 GB virtual disk `sda`, formatted as **ext4** (49G size, 13G used, 27%).

**Observe (2) – One `/dev` entry and what it is:**
`/dev/sda` = the whole virtual hard disk of the VM. The `b` at the start means it is a **block device** (it reads/writes in blocks). Another one: `/dev/null` is a `c` (character) device that throws away everything you write to it.

**Observe (3) – "Everything is a file":**
Disks (`/dev/sda`), terminals (`/dev/tty`), random numbers (`/dev/random`) and even running processes (`/proc/<pid>/status`) all look like files in the tree, so programs use the same open/read/write to talk to all of them.

---

## Optional (finished early)

**htop:** it was not installed at first (`which htop` found nothing), so I used `top -b -n 1`. After installing it (`htop 3.4.1`) I ran it. It shows a coloured bar for each of the 4 CPU cores and bars for Mem and Swap, and you can scroll and kill processes with the keys. Header text I captured:

```
Tasks: 149, 709 thr, 116 kthr; 1 running
Load average: 1.63 2.06 1.41
Uptime: 00:28:15
Mem[|||| 1.47G/3.32G]
```

htop counts differently from `ps`: 149 user processes plus 116 kernel threads (kthr) is about 265, close to the ~261 I got from `ps aux` a few minutes earlier. It also shows 709 threads in total.

**`strace -c ls`** (summary, top lines):

```
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ------------------
 30.10    0.001690         187         9           fstat
 20.18    0.001133          41        27           mmap
 18.20    0.001022        1021         1           execve
  6.84    0.000384          48         8           mprotect
  5.45    0.000306          27        11         1 openat
  ...
  3.12    0.000175          12        14           read
  0.98    0.000055          55         1           write
  0.30    0.000017           8         2           getdents64
------ ----------- ----------- --------- --------- ------------------
100.00    0.005614          40       138        12 total
```

Just to list a folder, `ls` made **138 system calls** (31 different kinds). Most of them are for loading the program and its libraries (`execve`, `mmap`, `openat`, `mprotect`). The real work is `getdents64` (read the directory entries, 2 calls) and `write` (print the names, 1 call).

---

## Closing synthesis

The OS manages four resources: **files**, **processes**, **memory** and **devices (I/O)**.
I saw files with `ls -l` (owner, group, rwx permissions), processes with `ps aux` / `cat /proc/<pid>/status` (PID, state), and memory with `free -h` (RAM, cache and swap).
I saw devices with `lsblk` / `df -h`, which showed the disk `/dev/sda` and its partition `/dev/sda2` mounted as `/`.
