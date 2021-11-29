#include "message_slot.h" 
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int channel_id, message_slot_fd, return_val, num_of_bytes_read;
    char buffer[MAX_MESSAGE_LENGTH];
    
    if (argc != 3) {
        perror("Number of cmd args is not 2");
	    exit(1);
    }

    char *file_path = argv[1];
    sscanf(argv[2],"%d",&channel_id);

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

    // Reading a message from the slot to buffer
    num_of_bytes_read = read(message_slot_fd, buffer, MAX_MESSAGE_LENGTH);
    if (num_of_bytes_read < 0) {
        perror("Can't read a message from message slot");
        exit(1);
    }

    // Closing the device
    return_val = close(message_slot_fd);
    if (return_val != 0) {
        perror("Can't close device file");
        exit(1);
    }

    // Printing the message to standard output
    return_val = write(1, buffer, num_of_bytes_read);
    if (return_val < 0) {
        perror("Can't print the message to standard output");
        exit(1);
    }

    exit(0);
}