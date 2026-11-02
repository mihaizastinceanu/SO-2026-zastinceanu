# Lab 5 – The File System with Your Own Hands (Linux)

Zastinceanu Mihai, FAF-243. The complete report with all figures is the .docx file.

## AIM OF THE WORK

### Purpose of the practical work

The purpose of the work is to check on a real Linux system the main ideas of the file system: the directory tree and the file types, the inodes behind the names, hard and symbolic links, permissions and the disks underneath [1]. No programming is involved: only commands, careful observation and short explanations. The work was done on the course virtual machine (Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic) as the ordinary user papa_bear, because the root user ignores most permission checks.

### Problem statement

The following tasks have to be solved:

- to identify the file types in the tree and to compare the sizes of system directories
- to observe inodes, hard links, symbolic links, link counts of directories and the difference between size and allocated space
- to test the permissions of files and directories, the umask, the setuid bit and the sticky bit
- to describe the disk, its partitions and the information that the kernel exposes in /proc
- to perform the optional experiments with magic numbers, setuid programs, deleted open files and a file system in a file

## 1 THE TREE AND THE FILE TYPES

### 1.1 Theoretical background

The first character printed by `ls -l` shows the type of a file: - for a regular file, d for a directory, l for a symbolic link, c for a character device and b for a block device [2]. Devices are accessed through files in /dev and are identified by a major and a minor number. The command file determines the type of a file from its content, and du shows the space used by a directory tree.

### 1.2 Work performed

The commands of the first part are shown in Figure 1.1, and the device files and the target of /bin in Figure 1.2.

*Figure 1.1 – The tree, the file types, /dev, file and du*

*Figure 1.2 – Block and character devices and the target of /bin*

### 1.3 Answers to the questions

Question 1. /bin is l (a symbolic link), /dev/null is c (a character device, major 1, minor 3), /etc is d (a directory), /etc/passwd is - (a regular file), and /tmp is d (a directory with the sticky bit, drwxrwxrwt).

Question 2. /bin is a symbolic link to usr/bin; `readlink -f /bin` prints /usr/bin. Ubuntu uses the merged /usr layout, so /bin, /sbin and /lib point into /usr.

Question 3. A character device is /dev/tty0 (crw------- root tty 4, 0); a block device is /dev/sda (brw-rw---- root disk 8, 0), the virtual disk of the machine.

Question 4. /usr occupies 5.6 GB and /var/log 63 MB, so /usr is about 90 times bigger.

## 2 INODES AND LINKS

### 2.1 Theoretical background

A file is described by an inode, which holds its type, owner, permissions, size, link count, timestamps and the addresses of its data blocks; the name is not part of the inode but an entry in a directory [3]. A hard link is another directory entry for the same inode, while a symbolic link is a separate small file that contains a path [4]. The data of a file is freed only when its link count becomes 0 and no process keeps it open.

### 2.2 Work performed

The file notes.txt was inspected with stat, linked with ln and then removed (Figure 2.1); a symbolic link was created and its target removed, and the link count of a new directory was observed (Figure 2.2). Finally the size and the allocated space of a small file and of a sparse file were compared (Figure 2.3).

*Figure 2.1 – stat, and a hard link that survives the removal of the first name*

*Figure 2.2 – A symbolic link that breaks, and the link count of directories*

*Figure 2.3 – Size is not space: a 2-byte file and a sparse 1 GB file*

### 2.3 Answers to the questions

Question 1. In the output of stat, "Inode: 532287" is the inode number and "Links: 1" is the number of names of the file. After `ln notes.txt hard.txt` both names show the same inode 532287 and the count 2.

Question 2. Both names point to the same inode, and neither of them is the original. rm calls unlink, which removes one directory entry and decrements the link count to 1; the inode and the data remain, so hard.txt can still be read.

Question 3. soft.txt is a separate file (inode 532293) whose content is the path hard.txt. When it is opened, the kernel follows the path, which no longer exists, so cat reports "No such file or directory". Its size is 8 bytes, the length of the text hard.txt.

Question 4. A new directory has 2 links: its name d in the parent directory and the entry . inside d. Each subdirectory contains the entry .., which points back to d, so two subdirectories add two links, and the count becomes 4.

Question 5. Space is allocated in whole blocks of 4096 bytes, so the 2-byte file occupies one block, 4.0K (8 sectors of 512 bytes). truncate only sets the size field and allocates no blocks; the 1 GB file is a sparse file whose holes read as zeros, so du shows 0.

## 3 PERMISSIONS

### 3.1 Theoretical background

Every file has three groups of permissions, for the owner, the group and the others, each with the bits r, w and x, written in octal as 4, 2 and 1 [5]. On a directory r allows reading the list of names and x allows passing through it to the entries. New files receive the permissions 666 and new directories 777 minus the bits of the umask. The setuid bit makes a program run with the identity of its owner, and the sticky bit on a directory allows only the owner of a file to delete it.

### 3.2 Work performed

The permissions of a file and the umask were tested first (Figure 3.1). For the directory box the results were predicted before running: with r only, ls works and cat fails; with x only, ls fails and cat works. Both predictions were confirmed (Figure 3.2).

*Figure 3.1 – chmod 600, 000 and u+r, and the umask*

*Figure 3.2 – The box experiment, the setuid passwd, the sticky /tmp and /etc/shadow*

### 3.3 Answers to the questions

Question 1. The modes are described in Table 3.1.

*Table 3.1 – Common permission modes and their uses*

| Mode | rwx string | Typical use |
|---|---|---|
| 600 | `rw-------` | private files, for example an SSH private key or s.txt |
| 644 | `rw-r--r--` | documents and configuration readable by everyone, for example /etc/passwd |
| 755 | `rwxr-xr-x` | programs and public directories, for example the files in /usr/bin |
| 700 | `rwx------` | a private directory or script, for example ~/.ssh |

Question 2. For the owner the kernel checks only the owner bits, which are ---, so reading is denied. Ownership gives only the right to change the mode, which `chmod u+r` used; after it cat worked.

Question 3. The umask is 0002. New files are created with 666 − 002 = 664 (-rw-rw-r--, new.txt) and new directories with 777 − 002 = 775 (drwxrwxr-x, newdir).

Question 4. The bit r on a directory allows reading the list of names (ls box printed f), but not accessing the entries (cat box/f was denied). The bit x allows passing through the directory and using a known name (cat box/f printed hi), but not listing it (ls box was denied).

Question 5. The s in -rwsr-xr-x of /usr/bin/passwd is the setuid bit: the program runs with the user id of its owner, root. passwd needs it to modify /etc/shadow, which has the mode -rw-r----- root shadow and cannot even be read by an ordinary user. The t in drwxrwxrwt of /tmp is the sticky bit: everyone can create files there, but only the owner of a file can delete or rename it.

## 4 DISKS AND THE KERNEL AS FILES

### 4.1 Theoretical background

A disk is divided into partitions, and each partition can hold a file system that is mounted into the single directory tree. The kernel also exposes its own data as files: /proc contains a directory for every process and files with information about the processor and the memory [6].

### 4.2 Work performed

The disk, the mounts, the processor and the memory are shown in Figure 4.1, and the files of /proc/self in Figure 4.2. The layout of the disk is summarised in Table 4.1.

*Figure 4.1 – lsblk, df, mount, the number of processors and the memory*

*Figure 4.2 – /proc/self/status and /proc/self/fd*

*Table 4.1 – Layout of the disk of the virtual machine*

| Device | Size | Type | Content |
|---|---|---|---|
| sda | 50 GB | disk | the VirtualBox virtual disk |
| sda1 | 1 MB | partition | BIOS boot partition of the boot loader, no file system |
| sda2 | 50 GB | partition | ext4 file system mounted at / |
| sr0 | 1024 MB | rom | the virtual CD/DVD drive |
| loop0 – loop21 | various | loop | read-only images of snap packages |

### 4.3 Answers to the questions

Question 1. The disk is sda (50 GB) with the partitions sda1 (1 MB, BIOS boot, no file system) and sda2 (50 GB), which is mounted at / with the file system ext4 (Table 4.1).

Question 2. The root file system /dev/sda2 has 49 GB, of which 14 GB are used and 33 GB are available, so it is 31% full.

Question 3. According to /proc the machine has 4 processor cores (grep -c processor gives 4) and 3479680 kB of RAM, about 3.3 GiB; the virtual machine is given 4096 MB, and the part reserved at boot is not counted.

Question 4. /proc/self/status showed "Name: head". /proc/self is a link that every process sees as its own directory /proc/<pid>, and the process that opened the file was head. In the same way `ls -l /proc/self/fd` showed the descriptors of ls: 0, 1 and 2 on the terminal /dev/pts/0 and 3 for the directory that ls was reading.

## 5 OPTIONAL EXPERIMENTS

### 5.1 Work performed

Magic numbers (Figure 5.1). The command file ignored the name notes.txt and read the first bytes: 1f 8b is the signature of gzip (08 means the deflate method), so the file was reported as gzip compressed data. An ELF executable starts with the bytes 7f 45 4c 46, which are the characters \x7fELF.

*Figure 5.1 – Magic numbers of a gzip file and of an ELF program*

setuid programs (Figure 5.2). The search found 17 setuid programs. Besides the usual ones (passwd, su, mount, umount, chsh, chfn, gpasswd, pkexec, fusermount3), this system has /usr/lib/cargo/bin/sudo and su, which belong to sudo-rs, the Rust implementation used by Ubuntu 26.04, the classic sudo renamed to sudo.ws, and vmware-user-suid-wrapper, ntfs-3g, pppd, ssh-keysign and dbus-daemon-launch-helper.

*Figure 5.2 – The setuid programs of the system*

A deleted open file (Figure 5.3). After rm the name bigfile was gone, but the descriptor 3 of the shell still pointed to "/home/papa_bear/os-lab5/bigfile (deleted)", and the used space stayed at 14681024 KB. Only after `exec 3<&-` closed the last reference did the used space decrease by 51200 KB, exactly the 50 MB of the file.

*Figure 5.3 – The space of a deleted file is freed only when it is closed*

A file system in a file (Figure 5.4). mkfs.ext4 created 16384 inodes in the image of 64 MB, of which 12 are used by the reserved inodes and lost+found; df shows a usable size of 56 MB, because the rest holds the metadata and the journal.

*Figure 5.4 – An ext4 file system in a 64 MB file, mounted through a loop device*

## CONCLUSIONS

The work showed that a file name is only a directory entry that points to an inode, and that the inode holds the real information about the file: type, owner, mode, size, link count and blocks. A hard link is a second name for the same inode, a symbolic link is a small file that contains a path, and rm only removes a name. Space is allocated in blocks, so the size of a file and the space it occupies differ in both directions, and an open file keeps its blocks even after its last name is removed.

The permissions are checked separately for the owner, the group and the others, and have different meanings for files and for directories. The setuid bit lets an ordinary user change a file of root in a controlled way, and the sticky bit makes a shared directory such as /tmp safe. The disks, the partitions and even the current process are visible as files in /dev and /proc.

## REFERENCES

1. The File System with Your Own Hands. Laboratory Work 5, Operating Systems course. FCIM, Technical University of Moldova, 2026.
2. SILBERSCHATZ, A., GALVIN, P. B., GAGNE, G. Operating System Concepts. 10th edition. Hoboken: Wiley, 2018.
3. inode(7). Linux manual page [online]. Available: https://man7.org/linux/man-pages/man7/inode.7.html
4. symlink(7). Linux manual page [online]. Available: https://man7.org/linux/man-pages/man7/symlink.7.html
5. chmod(1). Linux manual page [online]. Available: https://man7.org/linux/man-pages/man1/chmod.1.html
6. proc(5). Linux manual page [online]. Available: https://man7.org/linux/man-pages/man5/proc.5.html
