#include <stdio.h>
#include <fcntl.h>    // For open() flags like O_RDONLY, O_WRONLY, O_CREAT
#include <unistd.h>   // For read(), write(), and close() system calls
#include <string.h>

int main() {
    int fd;
    char ch;
    const char *filename = "r.txt";

    // 1. Open for Writing (O_WRONLY) or Create (O_CREAT) with permissions (0644)
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error opening file for writing");
        return 1;
    }

    printf("Enter characters (press Enter + Ctrl+D to stop):\n");
    // read(0, ...) reads from standard input (keyboard)
    while (read(0, &ch, 1) > 0) {
        write(fd, &ch, 1); // write() one character at a time to the file
    }
    close(fd);

    // 2. Open for Reading (O_RDONLY)
    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file for reading");
        return 1;
    }

    printf("\nContent of the file:\n");
    // read() returns 0 at the end of file (EOF)
    while (read(fd, &ch, 1) > 0) {
        printf("%c", ch);
    }

    close(fd);
    return 0;
}

