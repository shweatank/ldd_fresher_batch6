#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main() {
    struct winsize w;

    // fd: STDOUT_FILENO (standard output)
    // op: TIOCGWINSZ (operation to get window size)
    // arg: address of the winsize structure to fill
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl error");
        return 1;
    }

    printf("Your terminal is %d rows high and %d columns wide.\n", 
           w.ws_row, w.ws_col);

    return 0;
}

