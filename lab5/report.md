# Labs 5 and 6 – The File System (Linux) and Inside the xv6 File System

Zastinceanu Mihai, FAF-243. The full report with all screenshots is the .docx file in this folder.

## Purpose of the work

Lab 5: check on a real Linux system everything from the file-system lecture: the directory tree and the file types, inodes behind the names, hard and symbolic links, permissions (including setuid and the sticky bit), and the disks and the kernel exposed as files.

Lab 6: see the same ideas in the xv6 file system, which implements them in a few hundred readable lines: decode what xv6 `ls` prints, write a `stat` program (`mystat`), measure the limits written in `kernel/fs.h`, and write `find`, which walks the directory tree recursively.

Environment: the course VirtualBox VM, Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic, working as the normal user `papa_bear` (`whoami` does not print root); xv6-riscv commit `06aad25`, riscv64-linux-gnu-gcc 15.2.0, QEMU 10.2.1. Figures are screenshots of the VM terminal and are numbered lab.part.n (Figure 5.2.1 is Lab 5, Part 2, first figure).

## Lab 5. The File System with Your Own Hands (Linux)

### Part 1. The tree and the file types

*Figure 5.1.1 – The tree, file types, /dev, file and du (screenshot in the .docx report)*

*Figure 5.1.2 – Block and character devices; where /bin points (screenshot in the .docx report)*

**1. For each path in the second line, write its type from the first letter of ls -l.**

`/bin` – `l`, symbolic link; `/dev/null` – `c`, character device (major 1, minor 3); `/etc` – `d`, directory; `/etc/passwd` – `-`, regular file; `/tmp` – `d`, directory (`drwxrwxrwt`, with the sticky bit).

**2. What is /bin on your system: a directory or a link, and to where?**

A symbolic link: `/bin -> usr/bin` (`readlink -f /bin` → `/usr/bin`). Ubuntu uses the "merged /usr" layout, so `/bin`, `/sbin` and `/lib` all point into `/usr`. `file /bin` also says "symbolic link to usr/bin".

**3. Find one character device and one block device in /dev.**

Character: `/dev/tty0` (`crw------- root tty 4, 0`) or `/dev/null`. Block: `/dev/sda` (`brw-rw---- root disk 8, 0`), the virtual hard disk; `/dev/sda1`, `/dev/loop0` are block devices too.

**4. Which is bigger on your laptop, /var/log or /usr, and by roughly how much?**

`/usr` is 5.6G and `/var/log` is 63M, so `/usr` is about 90 times bigger.

### Part 2. Inodes and links

*Figure 5.2.1 – stat, and a hard link that survives rm of the original name (screenshot in the .docx report)*

*Figure 5.2.2 – A symbolic link that breaks, and the link count of directories (screenshot in the .docx report)*

*Figure 5.2.3 – Size is not space: a 2-byte file and a sparse 1 GB file (screenshot in the .docx report)*

**1. Which number in stat is the inode, and which tells how many names the file has?**

`Inode: 532287` is the inode number; `Links: 1` is the number of names (hard links). After `ln notes.txt hard.txt`, `ls -li` shows the same inode 532287 for both names and the count 2.

**2. After rm notes.txt, why could you still read hard.txt? What does rm really remove?**

Both names point to the same inode; neither is "the original". `rm` calls `unlink()`, which removes one directory entry (one name) and decrements the link count to 1. The inode and its data blocks are freed only when the count reaches 0 and no process has the file open, so `cat hard.txt` still printed `hello`.

**3. Why did cat soft.txt fail at the end, although soft.txt still exists? How big is soft.txt and why exactly that size?**

A symbolic link is a separate file (its own inode, 532293) whose content is just a path. Opening it makes the kernel follow the path `hard.txt`, which no longer exists, so `No such file or directory` (ls shows it in red as a dangling link). Its size is 8 bytes, the length of the text `hard.txt`.

**4. A new directory has 2 links, and 4 after you create two subdirectories. Explain where each link comes from.**

Link 1 is the name `d` in its parent; link 2 is the entry `.` inside `d`, which points to `d` itself. Each subdirectory contains `..`, which points back to `d`, so `x` and `y` add one link each: 4. `ls -lai d` shows `.` with the inode of `d` and `..` with the inode of the parent.

**5. Why does a 2-byte file use 4.0K, and a 1 GB file use 0?**

Disk space is given in whole blocks. ext4 here uses 4096-byte blocks, so the 2-byte `one.txt` takes one block = 8 sectors of 512 bytes = 4.0K. `truncate -s 1G` only sets the size field and allocates no blocks (a sparse file with a hole that reads as zeros), so `du` shows 0 and `stat` shows 0 blocks.

### Part 3. Permissions

*Figure 5.3.1 – chmod 600 / 000 / u+r and the umask (screenshot in the .docx report)*

*Figure 5.3.2 – The box experiment, setuid passwd, sticky /tmp and /etc/shadow (screenshot in the .docx report)*

Predictions for the box experiment (written before running): with `r` only, `ls box` works and `cat box/f` fails; with `x` only, `ls box` fails and `cat box/f` works. Both were correct.

**1. Write 600, 644, 755 and 700 as rwx strings, and give one real use for each.**

600 = `rw-------`: private files, e.g. an SSH private key or `s.txt` here. 644 = `rw-r--r--`: ordinary documents and configuration readable by everyone, e.g. `/etc/passwd`. 755 = `rwxr-xr-x`: programs and public directories, e.g. `/usr/bin/*`, `box` at the end. 700 = `rwx------`: a private directory or a personal script, e.g. `~/.ssh`.

**2. With chmod 000 you own the file but cannot read it. Why does ownership not help?**

For the owner the kernel checks only the owner bits, and they are `---`, so read is denied. Being the owner gives only the right to change the mode, which is what `chmod u+r` did; after it, `cat` worked. (Only root ignores these checks.)

**3. What is your umask, and how does it explain the permissions of new.txt and newdir?**

`0002`. New files are created with 666 minus the mask → 664 = `-rw-rw-r--` (new.txt); new directories with 777 minus the mask → 775 = `drwxrwxr-x` (newdir). The mask removes only write for "others".

**4. Explain the box experiment: what does r allow on a directory, and what does x allow?**

`r` lets you read the list of names: `ls box` printed `f`, but `cat box/f` was denied, because to reach a file inside you must pass through the directory. `x` lets you pass through (search) it and use a name you already know: `cat box/f` printed `hi`, but `ls box` gave "cannot open directory".

**5. What do the s in /usr/bin/passwd and the t in /tmp mean, and why does passwd need the s?**

`s` (`-rwsr-xr-x root`) is setuid: the program runs with the user ID of the file's owner, root, whoever starts it. passwd must change the password hash in `/etc/shadow`, which is `-rw-r----- root shadow` (I got Permission denied reading it), so without setuid a normal user could not change their password. `t` (`drwxrwxrwt`) is the sticky bit: everyone may create files in /tmp, but only a file's owner (or root) may delete or rename it.

### Part 4. Disks and the kernel as files

*Figure 5.4.1 – lsblk, df, mount, CPU count and memory (screenshot in the .docx report)*

*Figure 5.4.2 – /proc/self/status and /proc/self/fd (screenshot in the .docx report)*

**1. Draw your disk: disk name, partitions, which one is mounted at / and with what file system type.**

See the drawing below. One 50 GB virtual disk `sda`: `sda1` (1 MB, the BIOS boot partition for GRUB, no file system) and `sda2` (50 GB, ext4, mounted at `/`). There is also `sr0` (the virtual CD drive) and read-only `loop` devices for snap packages.

**2. How full is your root file system?**

`/dev/sda2` has 49G, 14G used, 33G available: 31% (lsblk -f: 29% / 32.4G free).

**3. How many CPU cores and how much RAM does your laptop have, according to /proc?**

`grep -c processor /proc/cpuinfo` → 4 cores; `MemTotal: 3479680 kB` ≈ 3.3 GiB (the VM is given 4096 MB; the kernel keeps part of it for itself and does not count it).

**4. /proc/self/status showed the name of a program. Which one, and why that one?**

`Name: head`. `/proc/self` is a link that each process sees as its own `/proc/<pid>`; the process that opened the file was `head`, so it saw its own status. In the same way `ls -l /proc/self/fd` showed the descriptors of `ls`: 0, 1, 2 on the terminal `/dev/pts/0` and 3 for the directory ls was reading.

```
sda   50G  disk  (/dev/sda, VirtualBox virtual disk)
├─sda1   1M  part  BIOS boot partition (no file system)
└─sda2  50G  part  ext4, UUID be4cfdd9-...  mounted at  /   (14G of 49G used, 31%)
sr0   1024M  rom   virtual CD/DVD drive
loop0..loop21      snap packages (squashfs, read-only)
```

### Stretch tasks (all four)

*Figure 5.5.1 – (a) Magic numbers: gzip and ELF (screenshot in the .docx report)*

(a) `file` ignores the name `notes.txt` and reads the first bytes: `1f 8b` is the gzip magic number (`08` = deflate), so it says "gzip compressed data, was passwd". An ELF executable starts with `7f 45 4c 46` = `\x7fELF`.

*Figure 5.5.2 – (b) setuid programs on the VM (screenshot in the .docx report)*

(b) 17 setuid programs. Besides the classic ones (passwd, su, mount, umount, chsh, chfn, gpasswd, pkexec, fusermount3), this Ubuntu 26.04 VM has some that are not on the usual list: `/usr/lib/cargo/bin/sudo` and `su` (the new Rust sudo-rs), `/usr/bin/sudo.ws` (the classic sudo, renamed), `vmware-user-suid-wrapper`, `ntfs-3g`, `pppd`, `ssh-keysign` and `dbus-daemon-launch-helper`.

*Figure 5.5.3 – (c) Deleting a file that is still open (screenshot in the .docx report)*

(c) After `rm bigfile` the name is gone, but descriptor 3 of the shell still points to it: `3 -> /home/papa_bear/os-lab5/bigfile (deleted)`. The used space stays the same (14681024 KB). Only after `exec 3<&-` closes the last reference does it drop by 51200 KB (exactly the 50 MB) to 14629824 KB.

*Figure 5.5.4 – (d) A 64 MB ext4 file system in a file, mounted with a loop device (screenshot in the .docx report)*

(d) mkfs.ext4 created 16384 inodes in the 64 MB image (`df -i`); 12 are already used (the reserved inodes and lost+found). `df -h` shows 56M of usable size: the rest holds the metadata and the journal.

## Lab 6. Inside the xv6 File System

### Part 1. Read what xv6 shows you

*Figure 6.1.1 – ls in the root of a fresh xv6 disk, mkdir a, ls a (screenshot in the .docx report)*

*Figure 6.1.2 – kernel/stat.h, struct dinode and the link-count lines in sysfile.c (screenshot in the .docx report)*

**1. What do types 1, 2 and 3 mean (kernel/stat.h)?**

`T_DIR 1` – directory, `T_FILE 2` – regular file, `T_DEVICE 3` – device (only `console` here).

**2. Which inode number does the root directory have, and why are . and .. the same there?**

Inode 1 (`ROOTINO 1` in kernel/fs.h). The root has no parent, so mkfs writes both `.` and `..` with inode 1; that is why both lines are `1 1 1024`.

**3. An empty directory a has size 32. How big is one entry, and which two entries does a contain?**

A `struct dirent` is `ushort inum` (2 bytes) + `char name[DIRSIZ]` (14 bytes) = 16 bytes. 32 bytes = two entries: `.` (inode 29, the directory itself) and `..` (inode 1, the root), as `ls a` shows.

### Part 2. Write mystat

```
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char *
typename(int t)
{
  switch (t) {
  case T_DIR:
    return "directory";
  case T_FILE:
    return "file";
  case T_DEVICE:
    return "device";
  }
  return "?";
}

int
main(int argc, char *argv[])
{
  struct stat st;

  for (int i = 1; i < argc; i++) {
    if (stat(argv[i], &st) < 0) {
      fprintf(2, "mystat: cannot stat %s\n", argv[i]);
      continue;
    }
    printf("%s: %s, inode %d, links %d, size %d bytes\n", argv[i],
           typename(st.type), st.ino, st.nlink, (int)st.size);
  }
  exit(0);
}
```

*Figure 6.2.1 – mystat on README, . and console; a hard link, then rm of README (screenshot in the .docx report)*

**1. Paste the outputs after ln and after rm. What changed and what did not?**

After `ln README r2`: `README: file, inode 2, links 2, size 2441 bytes` and the same line for `r2`. After `rm README`: `r2: file, inode 2, links 1, size 2441 bytes`, and `cat r2 | wc` still gives `48 336 2441`. Only the link count changed (1 → 2 → 1); the inode number, size and data stayed the same. (Here `.` shows links 2, not 1 as in the sheet, because `mkdir a` before it added `..` in `a`, which counts as a link to the root: `dp->nlink++; // for ".."` in `create()`.)

**2. Find the line in kernel/sysfile.c (sys_unlink) that decrements the link count. When are the data blocks freed?**

`sys_unlink()` starts at line 202; line 244 is `ip->nlink--;`, followed by `iupdate(ip)` and `iunlockput(ip)`. The blocks are not freed there. They are freed in `iput()` (kernel/fs.c, line 350): when the last in-memory reference is dropped (`ip->ref == 1`) and `ip->nlink == 0`, it calls `itrunc(ip)` to free the data blocks and marks the inode free. So, as on Linux, data disappears when there is no name left and no process has the file open.

**3. xv6 has no symbolic links and no permissions. Look at struct dinode in kernel/fs.h: which fields does an xv6 inode have?**

`short type`, `short major`, `short minor` (device numbers), `short nlink`, `uint size` and `uint addrs[NDIRECT+1]` (12 direct block numbers and 1 indirect). No owner, group, mode bits or timestamps, which is why xv6 has no permissions.

*Figure 6.2.2 – nlink-- in sys_unlink and the freeing condition in iput (screenshot in the .docx report)*

### Part 3. Measure the limits

*Figure 6.3.1 – The constants in fs.h and the nmeta line printed by mkfs (screenshot in the .docx report)*

*Figure 6.3.2 – A name longer than DIRSIZ (screenshot in the .docx report)*

**1. What does ls show for averyveryverylongname, and which constant explains it?**

`averyveryveryl` – only the first 14 characters. `DIRSIZ 14` is the size of the name field in a directory entry, so longer names are cut to 14 characters when the entry is written.

**2. Compute the largest file xv6 can hold.**

`BSIZE` = 1024 bytes, `NDIRECT` = 12, one indirect block holds `NINDIRECT` = 1024 / 4 = 256 block numbers. MAXFILE = 12 + 256 = 268 blocks, and 268 × 1024 = 274 432 bytes (268 KiB).

**3. From the nmeta line: how many blocks does the whole disk have, and how many are for the log, the inodes, and the data?**

`nmeta 47 (boot, super, log blocks 31, inode blocks 13, bitmap blocks 1) blocks 1953 total 2000`. The disk has 2000 blocks of 1 KB: 1 boot + 1 superblock + 31 log + 13 inode + 1 bitmap = 47 metadata blocks, and 1953 data blocks.

### Part 4. Write find

`find` starts from `user/ls.c`: open the directory, read `struct dirent` entries, build the full path, `stat` it. It skips free slots (`inum == 0`), never recurses into `.` and `..`, compares the last component with `strcmp`, and closes every directory before returning (only 16 open files per process). The stretch option `-n` prints the inode number next to each path.

```
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// find <dir> <name> [-n]: print every path under <dir> whose last
// component is <name>; with -n, also print the inode number.

int show_inum;

// Return the last component of path (the part after the last '/').
char *
lastname(char *path)
{
  char *p;

  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  return p + 1;
}

void
find(char *path, char *name)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (strcmp(lastname(path), name) == 0) {
    if (show_inum)
      printf("%s %d\n", path, st.ino);
    else
      printf("%s\n", path);
  }

  if (st.type != T_DIR) {
    close(fd);
    return;
  }

  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
    fprintf(2, "find: path too long\n");
    close(fd);
    return;
  }
  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue; // free slot
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0)
      continue; // never walk back up or loop on ourselves
    find(buf, name);
  }
  close(fd); // only 16 open files per process: close before returning
}

int
main(int argc, char *argv[])
{
  if (argc < 3 || argc > 4 || (argc == 4 && strcmp(argv[3], "-n") != 0)) {
    fprintf(2, "usage: find <dir> <name> [-n]\n");
    exit(1);
  }
  show_inum = (argc == 4);
  find(argv[1], argv[2]);
  exit(0);
}
```

*Figure 6.4.1 – find tests on a fresh disk, and the -n stretch option (screenshot in the .docx report)*

`find . r2` printed `./a/b/r2`, `./a/r2`, `./r2`, and `find a b` printed `a/b`, as expected. With `-n`: `./a/b/r2 31`, `./a/r2 32`, `./r2 3`. `mystat cat` shows inode 3 with 2 links, so only `./r2` is the same file as `cat` (it was made with `ln cat r2`); the other two are different files that just have the same name.

## Conclusions

Lab 5 showed that a file name is only a directory entry pointing to an inode, and that the inode holds the real information: type, owner, mode, size, link count, blocks. A hard link is a second name for the same inode, a symbolic link is a small file containing a path, and `rm` only removes a name. Space is counted in blocks, so size and space differ in both directions (a 2-byte file takes 4K, a sparse 1 GB file takes nothing), and an open file keeps its blocks even after its last name is gone.

Permissions are checked by class (owner, group, others) and mean different things for files and directories (`r` = list, `x` = pass through). setuid and the sticky bit solve two real problems: letting a user change a root-owned file in a controlled way, and sharing /tmp safely. Disks, partitions and even the current process (`/proc/self`) are visible as files.

Lab 6 found the same ideas in xv6 in a much smaller form: a directory is a file of 16-byte entries, an inode has type, nlink, size and 13 block addresses, `unlink` decrements nlink and `iput` frees the blocks when nothing refers to the inode any more. The constants in fs.h fix the limits (14-character names, 268 KiB files, 2000-block disk). Writing `mystat` and a recursive `find` used exactly the calls `ls` uses: open, read of dirents, stat/fstat and close.
