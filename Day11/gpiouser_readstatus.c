#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE "/dev/led_gpio"

int main() {
    int fd = open(DEVICE, O_RDWR);
    char state;

    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    while (1) {
        read(fd, &state, 1);   // read current state

        if (state == '0') {
            write(fd, "1", 1);   // turn ON
            printf("LED ON\n");
        } else {
            write(fd, "0", 1);   // turn OFF
            printf("LED OFF\n");
        }

        sleep(1);
    }

    close(fd);
    return 0;
}
