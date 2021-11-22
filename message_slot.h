#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#include <linux/ioctl.h>

// The major device number
#define MAJOR_NUM 240

// TODO understand if needed
// Sets the file descriptor's channel id
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)

#define DEVICE_NAME "message_slot"
//#define BUF_LEN 80
//#define DEVICE_FILE_NAME "simple_char_dev"
//#define SUCCESS 0

#endif