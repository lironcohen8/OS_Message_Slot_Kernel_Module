#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

// The major device number
#define MAJOR_NUM 240
#define DEVICE_NAME "message_slot"
#define MAX_MESSAGE_LENGTH 128
#define MAX_SLOTS_NUM 256

// Sets the file descriptor's channel id
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)

#endif