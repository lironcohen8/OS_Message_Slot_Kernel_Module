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

struct message_channel {
    unsigned int id = 0;
    char *msg = NULL; // TODO change to bytes?
    unsigned int message_length;
};

struct message_slot {
    unsigned int slot_minor;
    struct message_channel channels[];
};

static struct message_slot *slots[];
static int slots_cntr = 0;
static int channels_cntr = 0;

struct message_channel get_channel(unsigned int id) {
    for (int i = 0; i < channels_cntr; i++) {
        if (channels[i].id == id) {
            return channels[i];
        }
    }
    return NULL;
}

static int device_open(struct inode* inode, struct file* file) {
    unsigned int minor = inode.iminor();
    if (slots[minor] == NULL) { // Check if we created a data structure for the file
        struct message_slot slot = kmalloc(sizeof(struct message_slot), GFP_KERNEL);
        slot.slot_minor = minor;
        slot.channels = kmalloc(sizeof(struct message_channel), GFP_KERNEL); // TODO fix 
        
        slots[slots_cntr++] = &slot;
    }
    return 0;
}

static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset) {
    int id = file->private_data;
    int return_val = 0;

    // No channel has been set on fd
    if (id == NULL) {
        return -EINVAL;
    }

    struct message_channel channel = get_channel(id);

    // No message exists on the channel
    if (channel.msg == NULL) {
        return -EWOULDBLOCK;
    }

    // Writing from channel to buffer
    for(int i = 0; i < length && i < MAX_MESSAGE_LENGTH; i++)
    {
        return_val = put_user(channel.msg[i], &buffer[i]);
        if (return_val != 0) {
            return -EFAULT;
        }
    }
    return channel.message_length;
}

static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset) {
    int id = file->private_data;
    int return_val = 0;

    // No channel has been set on fd
    if (id == NULL) {
        return -EINVAL;
    }

    // Checking message length
    if (length == 0 || length > 128) {
        return -EMSGSIZE;
    }

    struct message_channel channel = get_channel(id);
    kfree(channel.msg);
    channel.msg = kmalloc(length, GFP_KERNEL);
    if (channel.msg == NULL) {
        return -ENOMEM;
    }

    // Writing from buffer to channel
    for(int i = 0; i < length && i < MAX_MESSAGE_LENGTH; i++)
    {
        return_val = get_user(channel.msg[i], &buffer[i]);
        if (return_val != 0) {
            return -EFAULT;
        }
    }

    return length;
}

static long device_ioctl(struct file* file, unsigned int ioctl_command, unsigned int channel_id) {

    if (ioctl_command == MSG_SLOT_CHANNEL && channel_id != 0) {
        // Setting desired channel id to current file
        file->private_data = (void*) channel_id;
    }

    // command was not MSG_SLOT_CHANNEL or channel_id was 0
    else {    
        return -EINVAL;
    }
}

static int device_release(struct inode* inode, struct file* file) {
    return 0;
}

static int init_module(void) {
    // TODO init dev struct
    // memset(&device_info, 0, sizeof(struct chardev_info));

    // Register driver with desired major number
    int major = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);

    // Negative values signify an error
    if (major < 0)
    {
        printk(KERN_ERR "%s registraion failed for %d\n", DEVICE_NAME, major);
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