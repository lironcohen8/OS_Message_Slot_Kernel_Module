#include "message_slot.h" 
#include <fcntl.h>      // open
#include <unistd.h>     // exit
#include <sys/ioctl.h>  // ioctl
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *file_path;
    int channel_id;
    int message_slot_fd;
    
    if (argc != 3) {
        perror("Number of cmd args is not 2");
	    exit(1);
    }

    file_path = argv[1];
    channel_id = (int)argv[2];

    // Opening message slot device file
    message_slot_fd = open("/dev/"DEVICE_NAME, O_RDWR);

    // Setting the channel id
    ioctl(message_slot_fd);

    // Reading a message from the slot to buffer
    read(message_slot_fd);

    // Closing the device
    release(message_slot_fd);

    // Printing the message to standard output
    write();

    exit(0);
}