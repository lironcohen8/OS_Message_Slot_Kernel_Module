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

MODULE_LICENSE("GPL");

#include "message_slot.h"

struct message_channel_node {
    unsigned int channel_id;
    char *msg; // TODO change to bytes?
    unsigned int message_length;
    struct message_channel_node *next;
};

struct message_slot {
    unsigned int slot_minor;
    struct message_channel_node *channel_node_head;
    struct message_channel_node *channel_node_tail;
};

static struct message_slot **slots; // array of pointers to slots struct

static struct message_channel_node *get_channel(struct message_slot* cur_slot, unsigned int id) {
    struct message_channel_node *temp = cur_slot->channel_node_head;
    while (temp != NULL) {
        if (temp->channel_id == id) {
            // printk("slot is %d, channel id is %d, message in channel is %s\n", cur_slot->slot_minor, id, temp->msg);
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

static int device_open(struct inode* inode, struct file* file) {
    unsigned int minor = iminor(inode);
    if (slots[minor] == NULL) { // Check if we created a data structure for the file
        struct message_slot *slot = (struct message_slot *) kmalloc(sizeof(struct message_slot), GFP_KERNEL);
        memset(slot, 0, sizeof(struct message_slot));

        slot->slot_minor = minor;
        slots[minor] = slot;
    }
    return 0;
}

static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset) {
    struct message_channel_node *channel;
    struct message_slot* cur_slot;
    int channel_id, return_val, i;

    // No channel has been set on fd
    if (file->private_data == NULL) {
        return -EINVAL;
    }

    cur_slot = slots[iminor(file->f_inode)];
    channel_id = (int)(long)(file->private_data);
    channel = get_channel(cur_slot, channel_id);

    // Channel does not exist
    if (channel == NULL) {
        return -ENODATA;
    }

    // No message exists on the channel
    if (channel->msg == NULL) {
        return -EWOULDBLOCK;
    }

    length = channel->message_length;
    // Writing from channel to buffer
    for(i = 0; i < length && i < MAX_MESSAGE_LENGTH; i++)
    {
        return_val = put_user(channel->msg[i], &buffer[i]);
        if (return_val != 0) {
            return -EFAULT;
        }
    }
    return channel->message_length;
}

static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset) {
    struct message_channel_node *channel;
    struct message_slot* cur_slot;
    int channel_id, return_val, i;

    // No channel has been set on fd
    if (file->private_data == NULL) {
        return -EINVAL;
    }

    // Checking message length
    if (length == 0 || length > MAX_MESSAGE_LENGTH) {
        return -EMSGSIZE;
    }

    cur_slot = slots[iminor(file->f_inode)];
    channel_id = (int)(long)(file->private_data);
    channel = get_channel(cur_slot, channel_id);

    // Channel does not exist
    if (channel == NULL) {
        return -ENODATA;
    }

    channel->msg = (char *) kmalloc(length, GFP_KERNEL);
    memset(channel->msg, 0, length);
    channel->message_length = length;

    // Writing from buffer to channel
    for(i = 0; i < length; i++)
    {
        return_val = get_user(channel->msg[i], &buffer[i]);
        if (return_val != 0) {
            return -EFAULT;
        }
    }
    return length;
}

static long device_ioctl(struct file* file, unsigned int ioctl_command, unsigned long channel_id) {
    struct message_channel_node* channel;
    struct message_slot* cur_slot;

    if (ioctl_command == MSG_SLOT_CHANNEL && channel_id != 0) {
        // Setting desired channel id to current file
        file->private_data = (void *) channel_id;
        cur_slot = slots[iminor(file->f_inode)];

        channel = get_channel(cur_slot, channel_id);
        if (channel == NULL) { // channel was not initialized yet
            channel = (struct message_channel_node*) kmalloc(sizeof(struct message_channel_node), GFP_KERNEL);
            memset(channel, 0, sizeof(struct message_channel_node));
            channel->channel_id = channel_id;

            if (cur_slot->channel_node_head == NULL) { // First channel in slot
                cur_slot->channel_node_head = channel;
                cur_slot->channel_node_tail = channel;
            }
            else { // Adding at the end of the linked list
                cur_slot->channel_node_tail->next = channel;
                cur_slot->channel_node_tail = channel;
            }
        }
        return 0;
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
    .owner	        = THIS_MODULE, 
    .open           = device_open,
    .read           = device_read,
    .write          = device_write,
    .unlocked_ioctl = device_ioctl,
    .release        = device_release,
    // TODO check about release and flush
};

static int start_module(void) {
    int major;
    printk("----------------started over---------\n");

    slots = (struct message_slot **) kmalloc(MAX_SLOTS_NUM * sizeof(struct message_slot *), GFP_KERNEL);
    memset(slots, 0, MAX_SLOTS_NUM * sizeof(struct message_slot *));

    // Register driver with desired major number
    major = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);

    // Negative values signify an error
    if (major < 0)
    {
        printk(KERN_ERR "%s registraion failed for %d\n", DEVICE_NAME, major);
        return major;
    }

    printk("init module\n");
    return 0;
}

static void end_module(void) {
    struct message_slot* cur_slot; 
    struct message_channel_node *cur_channel_node, *temp_node;
    int i;
    for (i = 0; i < MAX_SLOTS_NUM; i++) {
        cur_slot = slots[i];
        if (cur_slot != NULL) {
            cur_channel_node = cur_slot->channel_node_head;
            while (cur_channel_node != NULL) {
                temp_node = cur_channel_node;
                cur_channel_node = cur_channel_node->next;
                if (temp_node->msg != NULL) {
                    kfree(temp_node->msg);
                }
                kfree(temp_node);
            }
            kfree(slots[i]);
        }
    }
    kfree(slots);
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk("exit module\n");
}

module_init(start_module);
module_exit(end_module);