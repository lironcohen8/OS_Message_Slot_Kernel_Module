#include "message_slot.h" 
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int channel_id, message_slot_fd, return_val;
    
    if (argc != 4) {
        perror("Number of cmd args is not 3");
	    exit(1);
    }

    // Parsing arguments
    char *file_path = argv[1];
    sscanf(argv[2],"%d",&channel_id);
    char *message = argv[3];

    // Opening message slot device file
    message_slot_fd = open(file_path, O_RDWR);
    if(message_slot_fd < 0) {
        perror("Can't open device file");
        exit(1);
    }

    // Setting the channel id
    return_val = ioctl(message_slot_fd, MSG_SLOT_CHANNEL, channel_id);
    if (return_val != 0) {
        perror("Can't set the channel id");
        exit(1);
    }

    // Writing the message to the slot
    // TODO without /0
    return_val = write(message_slot_fd, message, strlen(message)-1);
    if (return_val != 0) {
        perror("Can't write the message to message slot");
        exit(1);
    }

    // Closing the device
    return_val = close(message_slot_fd);
    if (return_val != 0) {
        perror("Can't close device file");
        exit(1);
    }

    exit(0);
}