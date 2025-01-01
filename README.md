# Memefs

## Project Overview
### Introduction
The purpose of this project is to familiarize us with designing and implementing a filesystem by developing a Multimedia Embedded Memory Encapsulation Filesystem (MEMEFS).
Our filesystem will consist of read and write file system drivers. We have implemented the following functions for the read drivers: read, readdir, getdir, and open. 
For the write drivers, we have implemented write, truncate, unlink, and create.

### Contact
Jason appiah, jasona2@umbc.edu
Krishna Mukti Chaulagain, Krishna!@umbc.edu

### Installation and Setup

#### Setup
We will need a computer with a Debian virtual machine. Additionally, we need to install the FUSE libraries and development packages. You can do this by running the following commands:
sudo apt-get update
sudo apt-get install fuse3 libfuse3-3 libfuse3-dev
Follow the script from the original project documentation to install the project files in the virtual machine, which includes `mkmemefs.c` in user space. This file will be used to create a MEMEFS image.

#### Build and Compile
Makefile does not work but it contains the proper commads.
Please use `sudo` to run these programs in case there are permission issues.

**Building and compiling `mkmemefs`:**
- build_img: 
 
  gcc -o mkmemefs mkmemefs.c
     
- compile_img: 
 
  ./mkmemefs memefs.img "MYVOLUME"
     

**Building and compiling `memefs.c`:**
- We have defined that `memefs.img` will be our image; the path does not matter, so please ensure the image is called `memefs.img`.
- Build: 
 
  gcc -o memefs memefs.c `pkg-config fuse3 --cflags --libs`
     
- Mount: 
 
  mkdir -p /tmp/memefs
     
- Compile: 
 
  ./memefs /tmp/memefs  # Please use sudo if encounter any issues with Permissions
     
- Unmount: 

  fusermount -u /tmp/memefs
     
To run the makefile, simply use `make` with the corresponding command.# 

If any error occurs please unmount and try again with sudo

### User-Space Driver Programs

#### Driver Overview
User-space drivers are essential for testing system calls in our filesystem. Our userspace code reflects the combination of both read and write drivers for the FUSE MEMEFS system. The read driver handles the following functions:
- `getattr`: Retrieve file attributes.
- `readdir`: List directory contents.
- `open`: Open a file.
- `read`: Read data from a file.

The write driver handles the following:
- `create`: Create new files in the MEMEFS image.
- `unlink`: Delete files from the MEMEFS image.
- `write`: Write data to a file, including appending and overwriting.
- `truncate`: Adjust the size of a file.

#### Building and Running Drivers
To build and run the test programs, first compile `mkmemefs.c` to create the `memefs.img` image. Then, build `memefs.c`, mount it, and compile it to access our driver.

### Testing Strategy
We aimed to mimic the example provided by the professor in the video she posted. We executed similar commands to ensure that the output was correct.
#### Test Cases
Key test cases and scenarios include: 
----------------------------------------------------------------------------------------------------------------------
- Testing `readdir`
        ls -l /tmp/memefs/
- Testing `getdir`
        stat /tmp/memefs/myfile
- Testing `read`
          cat /tmp/memefs/myfile
- Testing `open`
         cat /tmp/memefs/myfile
- Testing `create`
         touch /tmp/memefs/myfile
- Testing `write`
         echo "Hello, world!" > /tmp/memefs/myfile
- Testing `truncate`
         truncate -s 0 /tmp/memefs/myfile
- Testing `unlink`
         rm /tmp/memefs/myfile

Other scenarios, same file name, truncating to size 0 or truncating but not doing anything,rm empty.
----------------------------------------------------------------------------------------------------------------------
jasonappiah@debian:~/project-3-krishnamukti-jasonak19/userSpace/driver$ echo "jasonwashere" > /tmp/memefs/jasonfile
jasonappiah@debian:~/project-3-krishnamukti-jasonak19/userSpace/driver$ cat /tmp/memefs/jasonfile
jasonwashere

jasonappiah@debian:~/project-3-krishnamukti-jasonak19/userSpace/driver$ ls /tmp/memefs
hellowrld  jasonfile  newfile  newfile  newfile  newfile  newfile  newfile  newfile  testfile  testfile  testfile3

jasonappiah@debian:~/project-3-krishnamukti-jasonak19/userSpace/driver$ stat /tmp/memefs/afile
  File: /tmp/memefs/afile
  Size: 0               Blocks: 0          IO Block: 4096   regular empty file
Device: 22h/34d Inode: 2           Links: 1
Access: (0644/-rw-r--r--)  Uid: ( 1000/jasonappiah)   Gid: ( 1000/jasonappiah)
Access: 2069-12-31 14:00:00.000000000 -0500
Modify: 2069-12-31 14:00:00.000000000 -0500
Change: 2024-11-27 09:59:16.000000000 -0500
 Birth: -

Another Run=
root@CMSC421:/tmp/memefs# echo "Radhe " > shyam.txt
root@CMSC421:/tmp/memefs# ls
shyam.txt
root@CMSC421:/tmp/memefs# cat shyam.txt
Radhe 
root@CMSC421:/tmp/memefs# ls shyam
ls: cannot access 'shyam': No such file or directory
root@CMSC421:/tmp/memefs# ls
shyam.txt
root@CMSC421:/tmp/memefs# stat shyam.txt
  File: shyam.txt
  Size: 7         	Blocks: 8          IO Block: 4096   regular file
Device: 801h/2049d	Inode: 5130512     Links: 1
Access: (0644/-rw-r--r--)  Uid: (    0/    root)   Gid: (    0/    root)
Access: 2024-11-28 01:21:15.856681407 -0500
Modify: 2024-11-28 01:20:49.675597410 -0500
Change: 2024-11-28 01:20:49.675597410 -0500
 Birth: 2024-11-28 01:20:49.675597410 -0500
root@CMSC421:/tmp/memefs# ls l
ls: cannot access 'l': No such file or directory
root@CMSC421:/tmp/memefs# ls -l
total 1
-rw-r--r-- 1 root root 7 Nov 28 01:20 shyam.txt
 ----------------------------------------------------------------------------------------------------------------------
jasonappiah@debian:~/project-3-krishnamukti-jasonak19/userSpace/driver$ ls /tmp/memefs
new  new3
jasonappiah@debian:~/project-3-krishnamukti-jasonak19/userSpace/driver$ rm /tmp/memefs/new
jasonappiah@debian:~/project-3-krishnamukti-jasonak19/userSpace/driver$ ls /tmp/memefs
new3 
----------------------------------------------------------------------------------------------------------------------
New Run
krishnamukti@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/userSpace/driver$   gcc -o memefs memefs.c `pkg-config fuse3 --cflags --libs` 
krishnamukti@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/userSpace/driver$   mkdir -p /tmp/memefs
krishnamukti@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/userSpace/driver$   ./memefs /tmp/memefs 
 
 Radhe Radhe
Argument count: 2
Mount point: /tmp/memefs
Image file: /usr/src/project3/project-3-krishnamukti-jasonak19/userSpace/driver/memefs.img
fopen: Permission denied

# As w etalked about please use sudo if any permission issues

krishnamukti@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/userSpace/driver$ sudo -s
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/userSpace/driver#   ./memefs /tmp/memefs
 
 Radhe Radhe
Argument count: 2
Mount point: /tmp/memefs
Image file: /usr/src/project3/project-3-krishnamukti-jasonak19/userSpace/driver/memefs.img
Image file opened successfully!
argv[0]: ./memefs
argv[1]: /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/userSpace/driver# 

// IN /tmp/memefs
root@CMSC421:/tmp/memefs# ls
hello.txt  kmc	myfile	radhe  ram  rams.txt  ram.txt  sa
root@CMSC421:/tmp/memefs# touch test.txt
root@CMSC421:/tmp/memefs# ls
hello.txt  kmc	myfile	radhe  ram  rams.txt  ram.txt  sa  test.txt
root@CMSC421:/tmp/memefs# stat test.txt
  File: test.txt
  Size: 0         	Blocks: 0          IO Block: 4096   regular empty file
Device: 23h/35d	Inode: 2           Links: 1
Access: (0644/-rw-r--r--)  Uid: (    0/    root)   Gid: (    0/    root)
Access: 2069-12-31 14:00:00.000000000 -0500
Modify: 2069-12-31 14:00:00.000000000 -0500
Change: 2024-11-28 00:16:39.000000000 -0500
 Birth: -
root@CMSC421:/tmp/memefs# ls -l
total 0
-rw-r--r-- 1 root root 0 Nov 29  1999 hello.txt
-rw-r--r-- 1 root root 0 Nov 29  1999 kmc
-rw-r--r-- 1 root root 0 Nov 29  1999 myfile
-rw-r--r-- 1 root root 0 Dec 31  2069 radhe
-rw-r--r-- 1 root root 0 Nov 29  1999 ram
-rw-r--r-- 1 root root 0 Nov 29  1999 rams.txt
-rw-r--r-- 1 root root 0 Nov 29  1999 ram.txt
-rw-r--r-- 1 root root 0 Nov 29  1999 sa
-rw-r--r-- 1 root root 0 Dec 31  2069 test.txt

root@CMSC421:/tmp/memefs# ls
hello.txt  kmc	myfile	radhe  ram  rams.txt  ram.txt  sa  test.txt
root@CMSC421:/tmp/memefs# rm ram
root@CMSC421:/tmp/memefs# ls
hello.txt  kmc	myfile	radhe  rams.txt  ram.txt  sa  test.txt
root@CMSC421:/tmp/memefs# rm kmc
root@CMSC421:/tmp/memefs# ls
hello.txt  myfile  radhe  rams.txt  ram.txt  sa  test.txt
root@CMSC421:/tmp/memefs# 


----------------------------------------------------------------------------------------------------------------------


### Troubleshooting

#### Common Issues
We encountered an issue where time discrepancies hindered file creation, prompting us to build a function to update the time, similar to what is done in `mkmemefs.c`.
Truncate has not been test but the logic seems correct our code used to run fully but the write acts up sometimes.We are not sure why but.
It worked but for some reason it stopped.If the code does not work restart the virtual machine,unmount recreate the image , follow all the steps and should work again.
We kept getting time stamp erros as well as not in directory error as well as Space full errors.They came out of nowhere since most of the time our code ran.
The transport endpoint is not connected.For most of these errors,  you can kill the process  using ps aux | grep memefs and kill -9 pid  (this might fix it ) but restarting the vm unmounting remounting and compiling again will solve the issue.
We tested it and found no issues coming up but these were some issues we encountered while building the project, we included these just in case.
Sometimes, write using cat or echo  or truncate might brick your terminal.

### References
https://man7.org/linux/man-pages/man2/lseek.2.html
https://www.tutorialspoint.com/c_standard_library/c_function_fwrite.htm
https://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm
https://github.com/libfuse/libfuse
https://www.youtube.com/watch?v=n2AAhiujAqs&t=1631s
https://www.youtube.com/watch?v=bbmWOjuFmgA
https://www.youtube.com/watch?v=eNCgv2v6ULU
https://www.youtube.com/watch?v=LZCILvr5tUk&t=250s
https://stackoverflow.com/questions/13247647/convert-integer-from-pure-binary-to-bcd
https://maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/
https://engineering.facile.it/blog/eng/write-filesystem-fuse/
https://www.fsl.cs.stonybrook.edu/docs/fuse/fuse-article-appendices.html
