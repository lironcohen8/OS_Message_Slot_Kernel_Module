#include "message_slot.h" 
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int channel_id, message_slot_fd, return_val;
    char buffer[MAX_MESSAGE_LENGTH];
    
    if (argc != 3) {
        perror("Number of cmd args is not 2");
	    exit(1);
    }

    char *file_path = argv[1];
    sscanf(argv[2],"%d",&channel_id);
    printf("parsed\n");

    // Opening message slot device file
    message_slot_fd = open(file_path, O_RDWR);
    if(message_slot_fd < 0) {
        perror("Can't open device file");
        exit(1);
    }
    printf("opened\n");

    // Setting the channel id
    return_val = ioctl(message_slot_fd, MSG_SLOT_CHANNEL, channel_id);
    if (return_val != 0) {
        perror("Can't set the channel id");
        exit(1);
    }
    printf("set\n");

    // Reading a message from the slot to buffer
    return_val = read(message_slot_fd, buffer, MAX_MESSAGE_LENGTH);
    if (return_val < 0) {
        perror("Can't read a message from message slot");
        exit(1);
    }
    printf("read\n");

    // Closing the device
    return_val = close(message_slot_fd);
    if (return_val != 0) {
        perror("Can't close device file");
        exit(1);
    }
    printf("closed\n");

    // Printing the message to standard output
    return_val = write(1, buffer, MAX_MESSAGE_LENGTH);
    printf("return val in wrote to console is %d\n", return_val);
    if (return_val < 0) {
        perror("Can't print the message to standard output");
        exit(1);
    }
    printf("wrote to console\n");

    exit(0);
}