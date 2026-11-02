# Lab 6 – Inside the xv6 File System

Zastinceanu Mihai, FAF-243. The complete report with all figures is the .docx file.

## AIM OF THE WORK

### Purpose of the practical work

The purpose of the work is to study the xv6 file system, which has the same ideas as ext4 (inodes, directories as tables, hard links and a journal) in a few hundred readable lines: to interpret what xv6 shows, to write a stat program, to measure the limits written in kernel/fs.h and to write find, which walks the directory tree recursively [1]. The work was done with xv6-riscv commit 06aad25 on the course virtual machine, with the cross compiler riscv64-linux-gnu-gcc 15.2.0 and QEMU 10.2.1.

### Problem statement

The following tasks have to be solved:

- to decode the output of ls in xv6: the types, the inode numbers and the sizes
- to write the program mystat and to follow the link count of a file through ln and rm
- to find the limits of the file system: the length of a name, the largest file and the layout of the disk
- to write the program find, which prints every path under a directory whose last component has a given name

## 1 READING WHAT XV6 SHOWS

### 1.1 Theoretical background

In xv6 a directory is a file that contains a table of entries of the type struct dirent: an inode number of 2 bytes and a name of DIRSIZ = 14 bytes. An entry with the inode number 0 is a free slot. The type of a file is stored in its inode and is one of the constants of kernel/stat.h [2].

### 1.2 Work performed

On a fresh disk, ls listed the root directory, and then the directory a was created and listed (Figure 1.1). Each line contains the name, the type, the inode number and the size. The relevant definitions are shown in Figure 1.2.

*Figure 1.1 – ls in the root of a fresh xv6 disk, mkdir a and ls a*

*Figure 1.2 – kernel/stat.h, struct dinode and the link count lines of sysfile.c*

### 1.3 Answers to the questions

Question 1. In kernel/stat.h, T_DIR = 1 is a directory, T_FILE = 2 is a regular file and T_DEVICE = 3 is a device, such as console.

Question 2. The root directory has the inode number 1 (ROOTINO in kernel/fs.h). The root has no parent, so mkfs writes both . and .. with the inode 1, and the two lines are identical.

Question 3. One entry has 16 bytes: 2 bytes for the inode number and 14 bytes for the name. The size 32 corresponds to two entries: . (inode 29, the directory itself) and .. (inode 1, the root).

## 2 WRITING MYSTAT

### 2.1 Theoretical background

The library function stat() opens a path and fills a struct stat with the fields dev, ino, type, nlink and size of its inode [2]. The link count nlink is incremented by link and decremented by unlink, and an inode is freed only when it has no names and no open references.

### 2.2 Work performed

The program user/mystat.c (Listing 2.1) prints the type, the inode number, the number of links and the size of every argument and reports an error on file descriptor 2 for a path that does not exist.

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
*Listing 2.1 – The program user/mystat.c*

The program was used to follow a hard link (Figure 2.1), and the code that changes the link count and frees the blocks was located in the kernel (Figure 2.2).

*Figure 2.1 – mystat on README, . and console; a hard link, then rm of README*

*Figure 2.2 – The decrement of nlink in sys_unlink and the freeing condition in iput*

### 2.3 Answers to the questions

Question 1. After `ln README r2` both names show "file, inode 2, links 2, size 2441 bytes". After `rm README`, r2 shows "inode 2, links 1, size 2441 bytes", and `cat r2 | wc` still prints 48 336 2441. Only the link count changed; the inode number, the size and the data stayed the same. The directory . shows 2 links instead of 1 because mkdir a created the entry .. in a, which is a link to the root (dp->nlink++ in create()).

Question 2. sys_unlink begins on line 202 of kernel/sysfile.c, and the link count is decremented on line 244 by `ip->nlink--;`. The data blocks are freed later, in iput() (kernel/fs.c, line 350): when the last reference is released (ref == 1) and nlink == 0, it calls itrunc(ip), which frees the blocks, and marks the inode as free.

Question 3. struct dinode contains the fields type, major, minor, nlink, size and addrs[NDIRECT + 1] (12 direct block addresses and one indirect). It has no owner, no group, no permission bits and no timestamps.

## 3 MEASURING THE LIMITS

### 3.1 Theoretical background

An xv6 inode addresses its data with NDIRECT = 12 direct block numbers and one indirect block, which holds NINDIRECT = BSIZE / 4 further block numbers. The disk image is created by mkfs, which prints its layout in the nmeta line [2].

### 3.2 Work performed

The constants were read from kernel/fs.h and the layout of the disk was printed during the build (Figure 3.1); then a directory with a long name was created in xv6 (Figure 3.2).

*Figure 3.1 – The constants of fs.h and the nmeta line printed by mkfs*

*Figure 3.2 – A name longer than DIRSIZ*

### 3.3 Answers to the questions

Question 1. ls shows averyveryveryl, only the first 14 characters of the name, because the name field of a directory entry has DIRSIZ = 14 bytes.

Question 2. With BSIZE = 1024 bytes, NDIRECT = 12 and NINDIRECT = 1024 / 4 = 256, the largest file is given by formula (3.1):

    MAXFILE × BSIZE = (12 + 1024 / 4) × 1024 = 268 × 1024 = 274 432 bytes    (3.1)

where 12 is the number of direct blocks, 1024 / 4 is the number of block addresses of 4 bytes in the indirect block, and 1024 is the size of a block. The largest file has 268 KiB.

Question 3. The line "nmeta 47 (boot, super, log blocks 31, inode blocks 13, bitmap blocks 1) blocks 1953 total 2000" shows a disk of 2000 blocks: 1 boot block, 1 superblock, 31 log blocks, 13 inode blocks and 1 bitmap block, which are 47 metadata blocks, and 1953 data blocks.

## 4 WRITING FIND

### 4.1 Theoretical background

A recursive traversal opens a directory, reads its entries, builds the full path of each entry and calls itself for the subdirectories. The entries . and .. must be skipped, otherwise the walk never ends, and every opened directory must be closed, because an xv6 process can have only 16 open files [2], [4].

### 4.2 Work performed

The program user/find.c (Listing 4.1) starts from the structure of user/ls.c. It skips the entries with inum == 0, compares the last component of each path with the searched name using strcmp, and closes every directory before returning. The optional third argument -n also prints the inode number.

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
*Listing 4.1 – The program user/find.c*

The test of the handout was run on a fresh disk (Figure 4.1). `find . r2` printed ./a/b/r2, ./a/r2 and ./r2, and `find a b` printed a/b. With -n the inode numbers are 31, 32 and 3; mystat shows that cat also has the inode 3 and 2 links, so only ./r2, created with `ln cat r2`, is the same file as cat.

*Figure 4.1 – find on a fresh disk, and the option -n*

## CONCLUSIONS

The xv6 file system shows in a small form the ideas of a real one: a directory is a file of 16-byte entries, an inode holds the type, the link count, the size and 13 block addresses, unlink only decrements the link count, and iput frees the blocks when nothing refers to the inode any more. The constants of kernel/fs.h fix the limits of the system: names of 14 characters, files of at most 268 KiB and a disk of 2000 blocks.

The programs mystat and find were written with the same calls that ls uses: open, read of directory entries, stat or fstat and close. The test with -n confirmed that a hard link is the same inode under another name, while files with the same name in different directories are different inodes.

## REFERENCES

1. Inside the xv6 File System. Laboratory Work 6, Operating Systems course. FCIM, Technical University of Moldova, 2026.
2. COX, R., KAASHOEK, F., MORRIS, R. xv6: a simple, Unix-like teaching operating system [online]. MIT, 2024. Available: https://pdos.csail.mit.edu/6.1810/2024/xv6/book-riscv-rev4.pdf
3. xv6 for RISC-V. Source code repository, MIT PDOS [online]. Available: https://github.com/mit-pdos/xv6-riscv
4. Lab: Xv6 and Unix utilities. MIT 6.1810 Operating System Engineering [online]. Available: https://pdos.csail.mit.edu/6.1810/2024/labs/util.html
