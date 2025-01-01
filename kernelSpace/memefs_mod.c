// Shree Ram
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fcntl.h>
#include <linux/err.h>
#include <linux/unistd.h>
#include <linux/types.h>

#define DEVICE_NAME "meme_bridge"
#define MOUNT_PATH "/tmp/memefs/"
#define FILENAME "testfile" 
 #define BUF_LEN 1000
 
static dev_t dev_num; 
static struct class *chardev_class;
static struct cdev chardev_cdev;
static char msg[BUF_LEN + 1]; 
// default path just in case
char  * path ="RadheShyam";
module_param(path,charp,S_IRUSR | S_IWUSR);
 // prototypes
static ssize_t memefs_read(struct file *file, char __user *buf, size_t len, loff_t *offset);
static int memefs_open(struct inode * inode, struct file * file); 
static int memefs_release(struct inode * inode, struct file * file); 
static ssize_t memefs_write(struct file * file, const char __user *buf, size_t len, loff_t *offset);

struct file_operations fops = {
    .read = memefs_read,
    .open = memefs_open,
    .write = memefs_write,
    .release = memefs_release,
    };
    
void display(void){
     printk(KERN_INFO "the file path is %s \n",path);
 }
 
static ssize_t memefs_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    printk(KERN_INFO "memefs: Read from device \n");
    int read = 0; 
    // msg is our path
    const char *curr_msg = msg; 
 
     // checks if end of messege
    if (!*(curr_msg + *offset)) {  
        *offset = 0;  
        return 0;  
    } 
   // increment the data 
    curr_msg += *offset; 
    // while loop until all the data is copied keep coping form kenerlspace buff to userspace
    while (len && *curr_msg) { 
        put_user(*(curr_msg++), buf++); 
        // update the length increase read count
        len--; 
        read++; 
    } 
    // updated the off set 
    *offset += read; 
    // returen read 
    return read;  
    
 
}

static int memefs_open(struct inode * inode, struct file * file){
    printk(KERN_INFO "memefs: opening from device\n");
  // this copies the argument passed in during run time to msg so we can use it in read and pass it to the user space 
        sprintf(msg,path); 
return 0;
}

static int memefs_release(struct inode * inode, struct file * file){
    
    printk(KERN_INFO "memefs: releasing from device \n");
return 0;
}

static ssize_t memefs_write(struct file * file, const char __user *buf, size_t len , loff_t *offset){
    printk(KERN_INFO "memefs: writing from device \n");
          //  just print it,just print the buf we get from userspace

        pr_alert("The MEMEFS.C wrote to this moduel: %s   \n",buf ); 
 
    return 0; 
  
}


static int memefs_init(void) {

    printk(KERN_INFO "Radhe Radhe! \n");
    printk(KERN_INFO "Hello, World! Kernel module: MEMEFS loaded.\n");
    display();
       // creating the bridge allocating using dynamic allocation since passed 0

   int ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "memefs: Failed to allocate a major number\n");
        return ret;
    }
    int major_num = MAJOR(dev_num);
    // registering device
    chardev_class = class_create(THIS_MODULE, DEVICE_NAME);
    
    if (IS_ERR(chardev_class)) {
        unregister_chrdev_region(dev_num, 1);
        //for debugging in case of error - jason
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(chardev_class);
    }
    // initalizing
    cdev_init(&chardev_cdev, &fops);
      ret = cdev_add(&chardev_cdev, dev_num, 1);
    if (ret < 0) {
         // Destroy the bridge

        device_destroy(chardev_class, dev_num);
        class_destroy(chardev_class);
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "Failed to add device to the system\n");
        return ret;
    }
        // creating
    device_create(chardev_class, NULL, dev_num, NULL, DEVICE_NAME);

    return 0;
}

static void memefs_exit(void) {
 // Destroy the bridge
        device_destroy(chardev_class, dev_num);
        class_destroy(chardev_class);
        unregister_chrdev_region(dev_num, 1); 
 
    
    printk(KERN_INFO "Goodbye, World! Kernel module: MEMEFS  unloaded.\n");
    printk(KERN_INFO "Radhe Radhe! \n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Krishna and Jason");
MODULE_DESCRIPTION("Kernel Module for MEMEFS");
MODULE_VERSION("1.0");
 
module_init(memefs_init);
module_exit(memefs_exit);
