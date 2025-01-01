MEMEFS Kernel Module Implementation 

Project Overview:
The goal of this project is to extend the memefs filesystem by implementing a kernel-space module for reading files. 
This project builds upon the functionalities developed in Parts 1 and 2 and leverages the memefs implementation to interact with the kernel. 
The kernel module will read file contents from the memefs image mounted in user space and print them to dmesg.
Contact
Jason Appiah: jasona2@umbc.edu
Krishna Mukti Chaulagain: Krishnc1@umbc.edu
Installation and Setup
Prerequisites

A computer with a Debian-based Linux virtual machine.
Install the required packages for kernel module development:
sudo apt-get update
sudo apt-get install build-essential linux-headers-$(uname -r)

PLEASE RUN ALL OF THESE COMMAND AFTER RUNNING  SUDO -S TO AVOID PERMISSION ISSUES

Building the Kernel Module
Navigate to the "kernelSpace" directory:
cd /path/to/repo/kernelSpace
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

- Make the kernel Module 
  make

- Compile the memefs: 
  ./memefs /tmp/memefs  # Please use sudo if encounter any issues with Permissions   
 

************************************************************************************************
By this point you have  already build the image compiled it ran fuse(memefs.c)
************************************************************************************************
Access a file within the mounted MEMEfs directory:

Add a sample file to /tmp/memefs or brows the file ;
echo "RadheShyam" > /tmp/memefs/radhe

If by chance you run into no space left error: 
just do ls /tmp/memefs  one or two time,and repeat the command that gave you this error,Not sure why but this fixes it

verify if the file is created by using:
ls /tmp/memefs       you should see all the files you have created

verify the file content using:
cat /tmp/memefs/filename
on this case 
cat /tmp/memefs/radhe


************************************************************************************************
This is where the kernel module gets added,you can do dmesg | tail to check for debugging
************************************************************************************************
Insert the module into the kernel:
// the directory is alredy configured to /tmp/memefs so just replace the filename with the desired filename without /
//Replace it with the exact name, the one you just created above

insmod memefs_mod.ko path="filename"
to go with the example above filename will be radhe

insmod memefs_mod.ko path="radhe"

Verify module insertion:
 ls /dev | grep meme_bridge

Then to trigger the Module to read please use:
cat /tmp/memefs/filename
on this case 
cat /tmp/memefs/radhe

The kernel module will read the file content from /tmp/memefs/filename and print it to the system logs.
View dmesg output:
dmesg | tail

Remove the kernel module:
 rmmod memefs_mod

Unmount the filesystem:
fusermount -u /tmp/memefs


----------------------------------------------------------------------------------------------------------------------
 Driver Overview
User-space drivers are essential for testing system calls in our filesystem. Our userspace code reflects the combination of both read and write drivers for the FUSE MEMEFS system. The read driver handles the following functions:
- `getattr`: Retrieve file attributes.
- `readdir`: List directory contents.
- `open`: Open a file.
- `read`: Read data from a file.

The read and its helper fucntions deal with communication with the moduekl.

The write driver handles the following:
- `create`: Create new files in the MEMEFS image.
- `unlink`: Delete files from the MEMEFS image.
- `write`: Write data to a file, including appending and overwriting.

Kernel Space:
Similarly to userSpace, there are open, read, write, and new one's release,init and exit functions.
-`init_memefs`:Initalizes the module and creates the bridge between kernelspace and userpace,the bridge name is meme_bridge
-`exit_memefs`:Destroys the bridge
-`open_memefs`:Open the bridge in a sense but mainly copies the path argument to msg parameter which is passed to the userspace 
-`read_memefs`:This is where the msg parameter is send to the userspace
-`write_memefs`:This is called when userspace writes back to the device,this function just print it
-`relese_memefs`:This is basically a close function that does nothing just prints for debugging.
----------------------------------------------------------------------------------------------------------------------
Test Cases
Key test cases and scenarios include: 
----------------------------------------------------------------------------------------------------------------------
Same as Part 1 and 2  
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
- Testing `unlink`
         rm /tmp/memefs/myfile

Part 3 testing
- Testing
If no path is provided a default path is given RadheShyam a file that does not exist,so any command you run on fuse it will normally work without any issue
ex:  insmod test.ko
cat /tmp/memefs/radhe
It will print the content of radhe in userpace

If a invalid path is provided,the fuse operation works as normal, the kernel module does not cause any issue
ex:  insmod test.ko path="doesnotexit"  this file does not exist
cat /tmp/memefs/radhe
It will print the content of radhe in userpace

If a valid path is provided but the fuse operation is called for another file kernel module does nothing
ex: insmod test.ko path="keshav"
ls /tmp/memefs
keshav	mohan  radhe  shiva  shyam
but cat /tmp/memefs/radhe is called 
It will print the content of radhe in userspace

If a valid path is provide and the fuse operation is called for that file then in kernel log dmesg the file output will be printed.
ex: insmod test.ko path="keshav"
ls /tmp/memefs
keshav	mohan  radhe  shiva  shyam
and cat /tmp/memefs/keshav
then the content of keshav is printed on userspace as well as kernelspace log use dmesg | tail to check

There are automatic check so that moduel does not get inserted  or removed twice.

ERRORS OR KNOWN BUGS

If by chance you run into no space left error: 
just do ls /tmp/memefs  one or two time,and repeat the command that gave you this error,Not sure why but this fixes it(This bug only occuers when run the first time once in a while in a new directory)
If for any reason the moduel is not able to be inserted the Moduel please do dmesg | tail this should give you errors
The error We have encountered is Invalid ELF header magic: != \x7fELF which is solved by copying the code of kernel module:memefs_mod.c 
removing the file  using rm memefs_mod.c
creating another memefs_mod.c and pasting the same old code.
We have no clue why that error occurs it only happened twice and this solved it.
Once in a while printing the content in kernel you will see some random characters 1 or 2 and If you cat again they will go away.
Other wise the program should run smoothly.

Remember again do SUDO -S before running any commands
And the file path passed to kernel moduel is just the filename\
-----------------------------------------------------------------------------------------
Simple Run following instructions as given above:part 3 file name is mmemefs_mod in this case
-----------------------------------------------------------------------------------------
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# ls
Makefile  memefs.c  memefs_mod.c  mkmemefs.c
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr#   gcc -o mkmemefs mkmemefs.c
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr#   ./mkmemefs memefs.img "MYVOLUME"
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr#   gcc -o memefs memefs.c `pkg-config fuse3 --cflags --libs`
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr#   mkdir -p /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# make
make -C /lib/modules/5.15.675.15.67-cmsc421project0-krishnc1+/build M=/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr modules
make[1]: Entering directory '/usr/src/linux-headers-5.15.675.15.67-cmsc421project0-krishnc1+'
  CC [M]  /usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs_mod.o
/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs_mod.c: In function ‘memefs_read’:
/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs_mod.c:46:5: warning: ISO C90 forbids mixed declarations and code [-Wdeclaration-after-statement]
   46 |     int read = 0;
      |     ^~~
/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs_mod.c: In function ‘memefs_init’:
/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs_mod.c:103:4: warning: ISO C90 forbids mixed declarations and code [-Wdeclaration-after-statement]
  103 |    int ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
      |    ^~~
/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs_mod.c:108:5: warning: ISO C90 forbids mixed declarations and code [-Wdeclaration-after-statement]
  108 |     int major_num = MAJOR(dev_num);
      |     ^~~
/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs_mod.c:108:9: warning: unused variable ‘major_num’ [-Wunused-variable]
  108 |     int major_num = MAJOR(dev_num);
      |         ^~~~~~~~~
  MODPOST /usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/Module.symvers
  CC [M]  /usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs_mod.mod.o
  LD [M]  /usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs_mod.ko
make[1]: Leaving directory '/usr/src/linux-headers-5.15.675.15.67-cmsc421project0-krishnc1+'
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# ./memefs /tmp/memefs
Argument count: 2
Mount point: /tmp/memefs
Image file: /usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs.img
Image file opened successfully!
argv[0]: ./memefs
argv[1]: /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# echo "RadheShyam" > /tmp/memefs/radhe
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr#  ls /tmp/memefs
radhe
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# cat /tmp/memefs/radhe
RadheShyam
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# insmod memefs_mod.ko path="radhe"
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr#  ls /dev | grep meme_bridge
meme_bridge
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# cat /tmp/memefs/radhe
RadheShyam
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# dmesg | tail
[  595.619050] Radhe Radhe! 
[ 1005.764618] Radhe Radhe! 
[ 1005.764623] Hello, World! Kernel module: MEMEFS loaded.
[ 1005.764623] the file path is radhe 
[ 1016.815521] memefs: opening from device
[ 1016.815531] memefs: Read from device 
[ 1016.815540] memefs: writing from device 
[ 1016.815541] The MEMEFS.C wrote to this moduel: RadheShyam
               Ȏ\x7f   
[ 1016.815674] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr#  rmmod memefs_mod
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# dmesg | tail
[ 1005.764623] Hello, World! Kernel module: MEMEFS loaded.
[ 1005.764623] the file path is radhe 
[ 1016.815521] memefs: opening from device
[ 1016.815531] memefs: Read from device 
[ 1016.815540] memefs: writing from device 
[ 1016.815541] The MEMEFS.C wrote to this moduel: RadheShyam
               Ȏ\x7f  (This is example of extra beign added out of no where) if we run the cat command a again this should not show up 
[ 1016.815674] memefs: releasing from device 
[ 1034.277592] Goodbye, World! Kernel module: MEMEFS  unloaded.
[ 1034.277595] Radhe Radhe! 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# fusermount -u /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# 

Here is the countuation of the terminal I alreay unmoutned but and removed the device but it not necessary to get rid of extra characters being attached.
The extra charecters are not part of the message as you can see it in the new line which is not part of the print,the code ends at /n.
So the kernel is adding it form somewhere but that how you fix it just run the cat command again,it has never occured in the second time.

...continues 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr#   mkdir -p /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# ./memefs /tmp/memefs
Argument count: 2
Mount point: /tmp/memefs
Image file: /usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr/memefs.img
Image file opened successfully!
argv[0]: ./memefs
argv[1]: /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr#  ls /tmp/memefs
radhe
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# cat /tmp/memefs/radhe
RadheShyam
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# insmod memefs_mod.ko path="radhe"
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# cat /tmp/memefs/radhe
RadheShyam
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# dmesg | tail
[ 1034.277595] Radhe Radhe! 
[ 1140.656020] Radhe Radhe! 
[ 1140.656023] Hello, World! Kernel module: MEMEFS loaded.
[ 1140.656024] the file path is radhe 
[ 1142.590006] memefs: opening from device
[ 1142.590015] memefs: Read from device 
[ 1142.590021] memefs: writing from device 
[ 1142.590021] The MEMEFS.C wrote to this moduel: RadheShyam
                  
[ 1142.590133] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/dr# 


-----------------------------------------------------------------------------------------
Simple Run with the error case 
-----------------------------------------------------------------------------------------
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# insmod test.ko
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 3249.664620] memefs: Read from device 
[ 3249.664628] memefs: writing from device 
[ 3249.664629] The MEMEFS.C wrote to this moduel: RadheMohan
                  
[ 3249.664680] memefs: releasing from device 
[ 3264.085044] Goodbye, World! Kernel module: MEMEFS  unloaded. (PREVIOUS RUN ENDED HERE)
[ 3264.085048] Radhe Radhe! 
[ 5859.125399] Radhe Radhe! 
[ 5859.125435] Hello, World! Kernel module: MEMEFS loaded.
[ 5859.125436] the file path is RadheShyam 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# insmod test.ko
insmod: ERROR: could not insert module test.ko: File exists
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 3249.664620] memefs: Read from device 
[ 3249.664628] memefs: writing from device 
[ 3249.664629] The MEMEFS.C wrote to this moduel: RadheMohan
                  
[ 3249.664680] memefs: releasing from device 
[ 3264.085044] Goodbye, World! Kernel module: MEMEFS  unloaded.
[ 3264.085048] Radhe Radhe! 
[ 5859.125399] Radhe Radhe! 
[ 5859.125435] Hello, World! Kernel module: MEMEFS loaded.
[ 5859.125436] the file path is RadheShyam 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/keshav
RadheMohan
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
                  
[ 3249.664680] memefs: releasing from device 
[ 3264.085044] Goodbye, World! Kernel module: MEMEFS  unloaded.
[ 3264.085048] Radhe Radhe! 
[ 5859.125399] Radhe Radhe! 
[ 5859.125435] Hello, World! Kernel module: MEMEFS loaded.
[ 5859.125436] the file path is RadheShyam 
[ 5956.481058] memefs: opening from device
[ 5956.481063] memefs: Read from device 
[ 5956.481173] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# 

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
FULL TEST RUN: , instead of memefs_mod my program is called test to avoid confusion.
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# ls
'?'	    edit.mod.c	 hello.ko      helloo.c        Makefile.save   mkmemefs.c       read_memefs.mod     test.ko
 c_file     edit.mod.o	 hello.mod     helloo.c.save   memefs	       modules.order    read_memefs.mod.c   test.mod
 c_file.c   edit.o	 hello.mod.c   HI	       memefs.c        Module.symvers   read_memefs.mod.o   test.mod.c
 edit.c     h		 hello.mod.o   HO	       memefs.c.save   Readme	        read_memefs.o	    test.mod.o
 edit.ko    h.c		 hello.o       h.txt	       memefs.img      read_memefs.c    test.c		    test.o
 edit.mod   hello.c	 helloo        Makefile        mkmemefs        read_memefs.ko   testfile.txt
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# make
make -C /lib/modules/5.15.675.15.67-cmsc421project0-krishnc1+/build M=/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace modules
make[1]: Entering directory '/usr/src/linux-headers-5.15.675.15.67-cmsc421project0-krishnc1+'
make[1]: Leaving directory '/usr/src/linux-headers-5.15.675.15.67-cmsc421project0-krishnc1+'
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace#   gcc -o memefs memefs.c `pkg-config fuse3 --cflags --libs`
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace#   mkdir -p /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# ./memefs /tmp/memefs 
Argument count: 2
Mount point: /tmp/memefs
Image file: /usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/memefs.img
Image file opened successfully!
argv[0]: ./memefs
argv[1]: /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# ls /tmp/memefs
mohan  radhe  shiva  shyam
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat mohan
cat: mohan: No such file or directory
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/mohan
Krishnaya Vasshudevaya Harah Parmatmane Pradetat Kalesha Nasaya Govindiya Namo Namaha
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# ls /dev | grep meme_bridge
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# insmod test.ko path="mohan"
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
               01:54:28.023279 main     Package type: LINUX_64BITS_GENERIC
[   11.913940] 01:54:28.036625 main     7.0.20 r163906 started. Verbose level = 0
[   11.922892] 01:54:28.045548 main     vbglR3GuestCtrlDetectPeekGetCancelSupport: Supported (#1)
[   11.984702] vboxsf: g_fHostFeatures=0x8000000f g_fSfFeatures=0x1 g_uSfLastFunction=29
[   11.984891] vboxsf: Successfully loaded version 7.0.20 r163906
[   11.985027] vboxsf: Successfully loaded version 7.0.20 r163906 on 5.15.675.15.67-cmsc421project0-krishnc1+ (LINUX_VERSION_CODE=0x50f43)
[   11.988563] 01:54:28.111231 automount vbsvcAutomounterMountIt: Successfully mounted 'Documents' on '/media/sf_Documents'
[ 1643.030788] Radhe Radhe! 
[ 1643.030791] Hello, World! Kernel module: MEMEFS loaded.
[ 1643.030792] the file path is mohan 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/mohan
Krishnaya Vasshudevaya Harah Parmatmane Pradetat Kalesha Nasaya Govindiya Namo Namaha
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[   11.988563] 01:54:28.111231 automount vbsvcAutomounterMountIt: Successfully mounted 'Documents' on '/media/sf_Documents'
[ 1643.030788] Radhe Radhe! 
[ 1643.030791] Hello, World! Kernel module: MEMEFS loaded.
[ 1643.030792] the file path is mohan 
[ 1670.639664] memefs: opening from device
[ 1670.639672] memefs: Read from device 
[ 1670.639679] memefs: writing from device 
[ 1670.639680] The MEMEFS.C wrote to this moduel: Krishnaya Vasshudevaya Harah Parmatmane Pradetat Kalesha Nasaya Govindiya Namo Namaha
                  
[ 1670.639777] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/radhe
Keshav
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 1643.030792] the file path is mohan 
[ 1670.639664] memefs: opening from device
[ 1670.639672] memefs: Read from device 
[ 1670.639679] memefs: writing from device 
[ 1670.639680] The MEMEFS.C wrote to this moduel: Krishnaya Vasshudevaya Harah Parmatmane Pradetat Kalesha Nasaya Govindiya Namo Namaha
                  
[ 1670.639777] memefs: releasing from device 
[ 1681.802209] memefs: opening from device
[ 1681.802214] memefs: Read from device 
[ 1681.803552] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# rmmod test
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 1670.639672] memefs: Read from device 
[ 1670.639679] memefs: writing from device 
[ 1670.639680] The MEMEFS.C wrote to this moduel: Krishnaya Vasshudevaya Harah Parmatmane Pradetat Kalesha Nasaya Govindiya Namo Namaha
                  
[ 1670.639777] memefs: releasing from device 
[ 1681.802209] memefs: opening from device
[ 1681.802214] memefs: Read from device 
[ 1681.803552] memefs: releasing from device 
[ 1687.603807] Goodbye, World! Kernel module: MEMEFS  unloaded.
[ 1687.603810] Radhe Radhe! 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# insmod test.ko path="radhe"
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
                  
[ 1670.639777] memefs: releasing from device 
[ 1681.802209] memefs: opening from device
[ 1681.802214] memefs: Read from device 
[ 1681.803552] memefs: releasing from device 
[ 1687.603807] Goodbye, World! Kernel module: MEMEFS  unloaded.
[ 1687.603810] Radhe Radhe! 
[ 1698.045452] Radhe Radhe! 
[ 1698.045456] Hello, World! Kernel module: MEMEFS loaded.
[ 1698.045457] the file path is radhe 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/radhe
Keshav
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/radhe
Keshav
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 1702.433762] memefs: writing from device 
[ 1702.433763] The MEMEFS.C wrote to this moduel: Keshav
               he   
[ 1702.433908] memefs: releasing from device 
[ 1702.714506] memefs: opening from device
[ 1702.714513] memefs: Read from device 
[ 1702.714519] memefs: writing from device 
[ 1702.714520] The MEMEFS.C wrote to this moduel: Keshav
                  
[ 1702.715084] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# stat /tmp/memefs/radhe
  File: /tmp/memefs/radhe
  Size: 7         	Blocks: 0          IO Block: 4096   regular file
Device: 23h/35d	Inode: 3           Links: 1
Access: (0644/-rw-r--r--)  Uid: (    0/    root)   Gid: (    0/    root)
Access: 1999-11-29 19:00:00.000000000 -0500
Modify: 1999-11-29 19:00:00.000000000 -0500
Change: 1999-11-29 19:00:00.000000000 -0500
 Birth: -
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# ls -l /tmp/memefs
total 0
-rw-r--r-- 1 root root 86 Nov 29  1999 mohan
-rw-r--r-- 1 root root  7 Nov 29  1999 radhe
-rw-r--r-- 1 root root 25 Nov 29  1999 shiva
-rw-r--r-- 1 root root  6 Nov 29  1999 shyam
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# echo "RadheMohan" > /tmp/memefs/keshav
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# ls -l /tmp/memefs
total 0
-rw-r--r-- 1 root root 11 Nov 29  1999 keshav
-rw-r--r-- 1 root root 86 Nov 29  1999 mohan
-rw-r--r-- 1 root root  7 Nov 29  1999 radhe
-rw-r--r-- 1 root root 25 Nov 29  1999 shiva
-rw-r--r-- 1 root root  6 Nov 29  1999 shyam
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# stat /tmp/memefs/keshav
  File: /tmp/memefs/keshav
  Size: 11        	Blocks: 0          IO Block: 4096   regular file
Device: 23h/35d	Inode: 6           Links: 1
Access: (0644/-rw-r--r--)  Uid: (    0/    root)   Gid: (    0/    root)
Access: 1999-11-29 19:00:00.000000000 -0500
Modify: 1999-11-29 19:00:00.000000000 -0500
Change: 1999-11-29 19:00:00.000000000 -0500
 Birth: -
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/keshav
RadheMohan
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# demsg | tail
bash: demsg: command not found
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 1702.433908] memefs: releasing from device 
[ 1702.714506] memefs: opening from device
[ 1702.714513] memefs: Read from device 
[ 1702.714519] memefs: writing from device 
[ 1702.714520] The MEMEFS.C wrote to this moduel: Keshav
                  
[ 1702.715084] memefs: releasing from device 
[ 1808.946763] memefs: opening from device
[ 1808.946771] memefs: Read from device 
[ 1808.946898] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# rmmod test
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# insmod test.ko path="keshav"
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
                  
[ 1702.715084] memefs: releasing from device 
[ 1808.946763] memefs: opening from device
[ 1808.946771] memefs: Read from device 
[ 1808.946898] memefs: releasing from device 
[ 1828.571032] Goodbye, World! Kernel module: MEMEFS  unloaded.
[ 1828.571036] Radhe Radhe! 
[ 1844.032005] Radhe Radhe! 
[ 1844.032010] Hello, World! Kernel module: MEMEFS loaded.
[ 1844.032011] the file path is keshav 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/keshav
RadheMohan
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 1828.571036] Radhe Radhe! 
[ 1844.032005] Radhe Radhe! 
[ 1844.032010] Hello, World! Kernel module: MEMEFS loaded.
[ 1844.032011] the file path is keshav 
[ 1850.801773] memefs: opening from device
[ 1850.801777] memefs: Read from device 
[ 1850.801781] memefs: writing from device 
[ 1850.801782] The MEMEFS.C wrote to this moduel: RadheMohan
                  
[ 1850.801874] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace#
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Another RUN
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace#   mkdir -p /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace#  ./memefs /tmp/memefs
Argument count: 2
Mount point: /tmp/memefs
Image file: /usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/memefs.img
Image file opened successfully!
argv[0]: ./memefs
argv[1]: /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# echo "Giridhar" > /tmp/memefs/ram.txt
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# ls /tmp/memefs
keshav	mohan  radhe  ram.txt  shiva  shyam
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/ram.txt
Giridhar
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace#  ls /dev | grep meme_bridge
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# insmod memefs_mod.ko path="ram.txt"
insmod: ERROR: could not load module memefs_mod.ko: No such file or directory
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# insmod test.ko path="ram.txt"
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace#  ls /dev | grep meme_bridge
meme_bridge
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/ram.txt
Giridhar
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 8490.101736] Radhe Radhe! 
[ 9160.866041] Radhe Radhe! 
[ 9160.866049] Hello, World! Kernel module: MEMEFS loaded.
[ 9160.866050] the file path is ram.txt 
[ 9175.037746] memefs: opening from device
[ 9175.037754] memefs: Read from device 
[ 9175.037760] memefs: writing from device 
[ 9175.037761] The MEMEFS.C wrote to this moduel: Giridhar
               \x06   
[ 9175.037781] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/ram.txt
Giridhar
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 9175.037760] memefs: writing from device 
[ 9175.037761] The MEMEFS.C wrote to this moduel: Giridhar
               \x06   
[ 9175.037781] memefs: releasing from device 
[ 9267.582083] memefs: opening from device
[ 9267.582090] memefs: Read from device 
[ 9267.582096] memefs: writing from device 
[ 9267.582096] The MEMEFS.C wrote to this moduel: Giridhar
                  
[ 9267.582226] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# 
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Another RUN
------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# make
make -C /lib/modules/5.15.675.15.67-cmsc421project0-krishnc1+/build M=/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace modules
make[1]: Entering directory '/usr/src/linux-headers-5.15.675.15.67-cmsc421project0-krishnc1+'
make[1]: Leaving directory '/usr/src/linux-headers-5.15.675.15.67-cmsc421project0-krishnc1+'
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace#   gcc -o memefs memefs.c `pkg-config fuse3 --cflags --libs`
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# fusermount -u /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# fusermount -u /tmp/memefs
fusermount: failed to unmount /tmp/memefs: Invalid argument
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace#   mkdir -p /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace#  ./memefs /tmp/memefs
Argument count: 2
Mount point: /tmp/memefs
Image file: /usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace/memefs.img
Image file opened successfully!
argv[0]: ./memefs
argv[1]: /tmp/memefs
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# ls /tmp/memefs
keshav	mohan  radhe  shiva  shyam
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/keshav
RadheMohan
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# insmod test.ko path="keshav"
insmod: ERROR: could not insert module test.ko: File exists
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# rmmod test
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# insmod test.ko path="keshav"
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 3179.803846] memefs: Read from device 
[ 3179.803861] memefs: writing from device 
[ 3179.803863] The MEMEFS.C wrote to this moduel: RadheMohan
               \x94\xff~   
[ 3179.803896] memefs: releasing from device 
[ 3210.382424] Goodbye, World! Kernel module: MEMEFS  unloaded.
[ 3210.382428] Radhe Radhe! 
[ 3212.407797] Radhe Radhe! 
[ 3212.407802] Hello, World! Kernel module: MEMEFS loaded.
[ 3212.407803] the file path is keshav 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# cat /tmp/memefs/keshav
RadheMohan
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 3210.382428] Radhe Radhe! 
[ 3212.407797] Radhe Radhe! 
[ 3212.407802] Hello, World! Kernel module: MEMEFS loaded.
[ 3212.407803] the file path is keshav 
[ 3249.664604] memefs: opening from device
[ 3249.664620] memefs: Read from device 
[ 3249.664628] memefs: writing from device 
[ 3249.664629] The MEMEFS.C wrote to this moduel: RadheMohan
                  
[ 3249.664680] memefs: releasing from device 
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# rmmod test
root@CMSC421:/usr/src/project3/project-3-krishnamukti-jasonak19/kernelSpace# dmesg | tail
[ 3212.407802] Hello, World! Kernel module: MEMEFS loaded.
[ 3212.407803] the file path is keshav 
[ 3249.664604] memefs: opening from device
[ 3249.664620] memefs: Read from device 
[ 3249.664628] memefs: writing from device 
[ 3249.664629] The MEMEFS.C wrote to this moduel: RadheMohan
                  
[ 3249.664680] memefs: releasing from device 
[ 3264.085044] Goodbye, World! Kernel module: MEMEFS  unloaded.
[ 3264.085048] Radhe Radhe! 

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

UPDATED the write function from part 2 due to an issue after submission here is the old version:We made it lot simpler, removed edge case that cause issues,made it more simple and easier to debug, If proj3 gets buggy or does not work please use this memefs.c file in this folder for testing,(the write is updated and read for part 3 but the rest is the same) or running everything except truncate is tested for and should work 100%.Thank you.

static int memefs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("memefs_write called for path: %s, size: %zu, offset: %ld\n", path, size, offset);

    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen");
        return -ENOENT;
    }

    const char *filename = path + 1;
    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        fclose(fs);
        return -EIO;
    }

    dir_entry entry;
    int found = 0;
    int index = 0;

    for (int i = 0; i < MAX_FILES; i++) {
        fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (i * sizeof(dir_entry)), SEEK_SET);
        fread(&entry, sizeof(dir_entry), 1, fs);

        if (strcmp(entry.filename, filename) == 0) {
            found = 1;
            // assigning entry
            index = i;
            break;
        }
    }

    if (!found) {
        fclose(fs);
        return -ENOENT;
    }

    uint16_t fat[BLOCK_SIZE / sizeof(uint16_t)];
    fseek(fs, superblock.main_fat * BLOCK_SIZE, SEEK_SET);
    fread(fat, sizeof(fat), 1, fs);
    //check if the offset is within the file size
    size_t end_offset = offset + size;
    size_t current_size = entry.file_size;
    size_t required_blocks = (end_offset + BLOCK_SIZE - 1) / BLOCK_SIZE;
    //extend the file
    if (end_offset > current_size) {
        uint16_t current_block = entry.start_block;
        while (fat[current_block] != 0xFFFF) {
            current_block = fat[current_block];
        }
        //check if there are enough free blocks
        size_t current_blocks = (current_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        size_t additional_blocks = required_blocks - current_blocks;
        for (size_t i = 0; i < additional_blocks; i++) {
            uint16_t free_block = 0xFFFF;
            for (uint16_t j = superblock.first_user_block; j < superblock.first_user_block + superblock.num_user_blocks;
                 j++) {
                if (fat[j] == 0x0000) {
                    free_block = j;
                    fat[j] = 0xFFFF;
                    break;
                }
            }

            if (free_block == 0xFFFF) {
                printf("No free blocks available during write\n");
                fclose(fs);
                return -ENOSPC;
            }

            fat[current_block] = free_block;
            current_block = free_block;
        }
    }

    uint16_t current_block = entry.start_block;
    size_t block_offset = offset / BLOCK_SIZE;
    size_t write_offset = offset % BLOCK_SIZE;
    //move to the block where the write should start
    for (size_t i = 0; i < block_offset; i++) {
        current_block = fat[current_block];
        if (current_block == 0xFFFF) {
            printf("Unexpected end of chain during write\n");
            fclose(fs);
            return -EIO;
        }
    }

    size_t bytes_written = 0;
    size_t remaining = size;
    //write the data
    while (remaining > 0) {
        size_t to_write = remaining > BLOCK_SIZE - write_offset ? BLOCK_SIZE - write_offset : remaining;

        fseek(fs, current_block * BLOCK_SIZE + write_offset, SEEK_SET);
        fwrite(buf + bytes_written, 1, to_write, fs);

        bytes_written += to_write;
        remaining -= to_write;
        write_offset = 0;

        if (remaining > 0) {
            current_block = fat[current_block];
            if (current_block == 0xFFFF) {
                printf("Unexpected end of chain during write\n");
                fclose(fs);
                return -EIO;
            }
        }
    }
    //update the file size if the end offset is greater than the current file size
    if (end_offset > entry.file_size) {
        entry.file_size = end_offset;
        fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (index) * sizeof(dir_entry), SEEK_SET);
        fwrite(&entry, sizeof(dir_entry), 1, fs);
        //update the last write timestamp
        time_t now = time(NULL);
        struct tm *tm_info = gmtime(&now);
        time_to_bcd(&now, entry.last_write);
    }

    fseek(fs, superblock.main_fat * BLOCK_SIZE, SEEK_SET);
    fwrite(fat, sizeof(fat), 1, fs);

    fclose(fs);

    printf("Write completed: %zu bytes written\n", bytes_written);
    return bytes_written;
}

This is the update one:

static int memefs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    printf("DEBUG: Write operation starting - path: %s, size: %zu, offset: %ld\n", path, size, offset);

    FILE *fs = fopen(image_path, "r+b");
    if (!fs) {
        perror("fopen failed");
        return -ENOENT;
    }

    const char *filename = path + 1;
    memefs_superblock_t superblock;
    if (load_superblock(fs, &superblock) != 0) {
        printf("DEBUG: Failed to load superblock\n");
        fclose(fs);
        return -EIO;
    }

    dir_entry entry;
    int entry_index = -1;

    // Find the file entry
    for (int i = 0; i < MAX_FILES; i++) {
        fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (i * sizeof(dir_entry)), SEEK_SET);
        if (fread(&entry, sizeof(dir_entry), 1, fs) != 1) {
            printf("DEBUG: Failed to read directory entry %d\n", i);
            fclose(fs);
            return -EIO;
        }

        if (strcmp(entry.filename, filename) == 0) {
            entry_index = i;
            break;
        }
    }

    if (entry_index == -1) {
        printf("DEBUG: File not found: %s\n", filename);
        fclose(fs);
        return -ENOENT;
    }

    printf("DEBUG: Found file entry at index %d\n", entry_index);

    if (entry.start_block < superblock.first_user_block ||
        entry.start_block >= superblock.first_user_block + superblock.num_user_blocks) {
        printf("DEBUG: Invalid start block: %d\n", entry.start_block);
        fclose(fs);
        return -EIO;
    }

    // Write the data directly to the first block
    fseek(fs, entry.start_block * BLOCK_SIZE + offset, SEEK_SET);
    size_t bytes_written = fwrite(buf, 1, size, fs);

    if (bytes_written != size) {
        printf("DEBUG: Write failed - expected %zu bytes, wrote %zu bytes\n", size, bytes_written);
        fclose(fs);
        return -EIO;
    }

    // Update file size if necessary
    if (offset + bytes_written > entry.file_size) {
        entry.file_size = offset + bytes_written;
        fseek(fs, (superblock.directory_start * BLOCK_SIZE) + (entry_index * sizeof(dir_entry)), SEEK_SET);
        if (fwrite(&entry, sizeof(dir_entry), 1, fs) != 1) {
            printf("DEBUG: Failed to update directory entry\n");
            fclose(fs);
            return -EIO;
        }
    }

    // Ensure all writes are flushed
    fflush(fs);
    fclose(fs);

    printf("DEBUG: Write completed successfully - %zu bytes written\n", bytes_written);
    return bytes_written;
}

Resources used:
*    https://sysprog21.github.io/lkmpg/ THIS HELPED A LOT LINUX PROGRAMMING GUIDE PROVIDED TO US 
*    https://www.youtube.com/watch?v=UdeGTRvPjK4&list=PL16941B715F5507C5&index=10 for c file separate testing 
*    https://www.youtube.com/watch?v=-O6GsrmOUgY&list=PLiTaEFNtJETPk3m0nMa8acegsaLE4ecWM
*    https://www.youtube.com/watch?v=DZrb9oSEzlU&list=PLCGpd0Do5-I3b5TtyqeF1UdyD4C-S-dMa
*    https://www.youtube.com/watch?v=ID3gjDflqyA
*    https://www.geeksforgeeks.org/how-to-fix-the-no-space-left-on-device-error-in-linux/
*    https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/

Thank you!
Radhe Radhe !
