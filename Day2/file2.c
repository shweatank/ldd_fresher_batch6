#include <stdio.h>

int main() {
    FILE *fp;
    char ch;

    // 1. Open for writing ("w")
    fp = fopen("r.txt", "w");
    if (fp == NULL) return 1;

    printf("Enter text (press Enter then Ctrl+D/Ctrl+Z to stop):\n");
    while ((ch = getchar()) != EOF) {
        fputc(ch, fp);
    }
    fclose(fp);

    // 2. Open for reading ("r")
    fp = fopen("r.txt", "r");
    if (fp == NULL) return 1;

    printf("\nContent of file:\n");
    while ((ch = fgetc(fp)) != EOF) {
        putchar(ch);
    }

    fclose(fp);
    return 0;
}

