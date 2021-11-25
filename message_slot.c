#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/ioctl.h>

#include "message_slot.h"

struct message_channel_node {
    unsigned int id;
    char *msg; // TODO change to bytes?
    unsigned int message_length;
    struct message_channel_node *next;
};

struct message_slot {
    unsigned int slot_minor;
    struct message_channel_node channel_node_head;
    struct message_channel_node channel_node_tail;
};

static struct message_slot **slots;
struct message_slot *cur_slot;

struct message_channel_node get_channel(unsigned int id) {
    struct message_channel_node *temp = &(cur_slot->channel_node_head);
    while (temp != NULL) {
        if (temp->id == id) {
            return *temp;
        }
        temp = temp->next;
    }
    return NULL;
}

static int device_open(struct inode* inode, struct file* file) {
    unsigned int minor = iminor(inode);
    if (slots[minor] == NULL) { // Check if we created a data structure for the file
        struct message_slot slot = kmalloc(sizeof(struct message_slot), GFP_KERNEL);
        memset(&slot, 0, sizeof(struct message_slot));

        slot.slot_minor = minor;
        slots[minor] = &slot;
        cur_slot = &slot;
    }
    return 0;
}

static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset) {
    struct message_channel_node channel;
    int id = file->private_data;
    int return_val = 0;

    // No channel has been set on fd
    if (id == NULL) {
        return -EINVAL;
    }

    channel = get_channel(id);

    // Channel does not exist
    if (channel == NULL) {
        return -ENODATA;
    }

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
    struct message_channel_node channel;
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

    channel = get_channel(id);

    // Channel does not exist
    if (channel == NULL) {
        return -ENODATA;
    }

    // Allocating new memory for message
    kfree(channel.msg);
    channel.msg = kmalloc(length, GFP_KERNEL);
    memset(&(channel.msg), 0, sizeof(char *));
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
    struct message_channel_node channel;
    if (ioctl_command == MSG_SLOT_CHANNEL && channel_id != 0) {
        // Setting desired channel id to current file
        file->private_data = (void*) channel_id;

        channel = get_channel(id);
        if (channel == NULL) { // channel was not initialized yet
            channel = kmalloc(sizeof(struct message_channel_node), GFP_KERNEL);
            memset(&channel, 0, sizeof(struct message_channel_node));

            if (cur_slot->channel_node_head == NULL) { // First channel in slot
                cur_slot->channel_node_head = channel;
                cur_slot->channel_node_tail = channel;
            }
            else { // Adding at the end of the linked list
                cur_slot->channel_node_tail.next = &channel;
                cur_slot->channel_node_tail = channel;
            }
        }
    }

    // command was not MSG_SLOT_CHANNEL or channel_id was 0
    else {    
        return -EINVAL;
    }
}

static int device_release(struct inode* inode, struct file* file) {
    return 0;
}

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

static int __init init_module(void) {
    int major;

    // TODO init dev struct
    slots = kmalloc(sizeof(struct message_slot **), GFP_KERNEL);
    memset(&slots, 0, sizeof(struct message_slot **));

    // Register driver with desired major number
    major = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);

    // Negative values signify an error
    if (major < 0)
    {
        printk(KERN_ERR "%s registraion failed for %d\n", DEVICE_NAME, major);
        return major;
    }

    printk("Loaded Module\n");
    return 0;
}

static void __exit exit_module(void) {
    // TODO free memory using kfree()
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk("Unloading Module\n");
}

module_init(init_module);
module_exit(exit_module);