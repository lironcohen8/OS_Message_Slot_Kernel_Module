#include "message_slot.h" 
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *file_path, message;
    int channel_id, message_slot_fd, return_val;
    
    if (argc != 4) {
        perror("Number of cmd args is not 3");
	    exit(1);
    }

    file_path = argv[1];
    channel_id = (int)argv[2];
    message = argv[3];

    // Opening message slot device file
    message_slot_fd = open("/dev/"DEVICE_NAME, O_RDWR);
    if(message_slot_fd < 0) {
        perror("Can't open device file: %s", DEVICE_NAME);
        exit(1);
    }

    // Setting the channel id
    return_val = ioctl(message_slot_fd);
    if (return_val != 0) {
        perror("Can't set the channel id");
        exit(1);
    }

    // Writing the message to the slot
    // TODO without /0
    return_val = write(message_slot_fd);
    if (return_val != 0) {
        perror("Can't write the message to message slot");
        exit(1);
    }

    // Closing the device
    return_val = release(message_slot_fd);
    if (return_val != 0) {
        perror("Can't close device file: %s", DEVICE_NAME);
        exit(1);
    }

    exit(0);
}