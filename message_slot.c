#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
// #include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include "message_slot.h"

struct file_operations Fops =
{
    .owner	  = THIS_MODULE, 
    .open     = device_open,
    .read     = device_read,
    .write    = device_write,
    .ioctl    = device_ioctl,
    .release  = device_release,
    // TODO check about release and flush
};

static int device_open(struct inode* inode,
                    struct file* file) {
}

static ssize_t device_read(struct file* file,
                           char __user* buffer,
                           size_t       length,
                           loff_t*      offset) {

    // No channel has been set on fd
    if (file->private_data == NULL) {
        return -EINVAL;
    }
}

static ssize_t device_write(struct file*       file,
                            const char __user* buffer,
                            size_t             length,
                            loff_t*            offset) {

    // No channel has been set on fd
    if (file->private_data == NULL) {
        return -EINVAL;
    }

    // Checking message length
    if (length == 0 || length > 128) {
        return -EMSGSIZE;
    }

    // TODO add writing to data structure
}

static long device_ioctl( struct file*   file,
                          unsigned int   ioctl_command,
                          unsigned int   channel_id) {

    if (ioctl_command == MSG_SLOT_CHANNEL && channel_id != 0) {
        // Setting desired channel id to current file
        file->private_data = (void*) channel_id;
    }

    // command was not MSG_SLOT_CHANNEL or channel_id was 0
    else {    
        return -EINVAL;
    }
}

static int device_release(struct inode* inode,
                          struct file* file) {
}

static int init_module(void) {
    // init dev struct
    // memset(&device_info, 0, sizeof(struct chardev_info));

    // Register driver with desired major number
    int major = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);

    // Negative values signify an error
    if (major < 0)
    {
        printk(KERN_ERR "%s registraion failed for %d\n", DEVICE_FILE_NAME, major);
        return major;
    }

    printk("Loaded Module\n");
    return 0;
}

static void exit_module(void) {
    // TODO free memory using kfree()
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk("Unloading Module\n");
}

module_init(init_module);
module_exit(exit_module);