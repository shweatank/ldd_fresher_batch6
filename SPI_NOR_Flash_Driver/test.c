#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

#define MAX_DATA 1024

static void print_hex_text(const uint8_t *buf, size_t len, uint32_t addr)
{
    size_t i;

    printf("Hex:\n");

    for (i = 0; i < len; i++) {
        if (i % 16 == 0)
            printf("%08lx: ", (unsigned long)(addr + i));

        printf("%02x ", buf[i]);

        if (i % 16 == 15)
            printf("\n");
    }

    if (len % 16)
        printf("\n");

    printf("DATA : \n");

    for (i = 0; i < len; i++) {
        if (buf[i] >= 32 && buf[i] <= 126)
            printf("%c", buf[i]);
        else
            printf(".");
    }

    printf("\n");
}

static int get_mtd_info(int fd, struct mtd_info_user *info)
{
    if (ioctl(fd, MEMGETINFO, info) < 0) {
        perror("MEMGETINFO");
        return -1;
    }

    printf("MTD Information\n");
    printf("-------------------------\n");
    printf("Size       : %u bytes\n", info->size);
    printf("Erase size : %u bytes\n", info->erasesize);
    printf("Write size : %u bytes\n", info->writesize);
    printf("Type       : %u\n\n", info->type);

    return 0;
}

static int erase_sector(int fd, uint32_t address,
                        struct mtd_info_user *info)
{
    struct erase_info_user erase;

    erase.start = address;
    erase.length = info->erasesize;

    printf("Erasing sector at 0x%08x...\n", address);

    if (ioctl(fd, MEMERASE, &erase) < 0) {
        perror("MEMERASE");
        return -1;
    }

    printf("Erase successful\n");

    return 0;
}

static int read_data(int fd, uint32_t address, size_t len)
{
    uint8_t *buf;
    ssize_t ret;

    buf = malloc(len);

    if (!buf) {
        perror("malloc");
        return -1;
    }

    if (lseek(fd, address, SEEK_SET) < 0) {
        perror("lseek");
        free(buf);
        return -1;
    }

    ret = read(fd, buf, len);

    if (ret < 0) {
        perror("read");
        free(buf);
        return -1;
    }

    printf("Read %ld bytes from 0x%08x\n", ret, address);
    
    print_hex_text(buf, ret, address);

    free(buf);

    return 0;
}

static int write_data(int fd, uint32_t address,
                      const uint8_t *data, size_t len)
{
    ssize_t ret;

    if (lseek(fd, address, SEEK_SET) < 0) {
        perror("lseek");
        return -1;
    }

    ret = write(fd, data, len);

    if (ret < 0) {
        perror("write");
        return -1;
    }

    if (ret != (ssize_t)len) {
        printf("Short write: %ld/%zu bytes\n", ret, len);
        return -1;
    }

    printf("Written %ld bytes at address 0x%08x\n",
           ret, address);

    return 0;
}

static int modify_data(int fd, uint32_t address,
                       const uint8_t *new_data, size_t new_len,
                       struct mtd_info_user *info)
{
    uint32_t sector_start;
    uint32_t offset;
    uint8_t *sector;
    uint8_t *verify;
    size_t sector_size;
    size_t i;

    sector_size = info->erasesize;

    sector_start = address -
                   (address % sector_size);

    offset = address - sector_start;

    if (offset + new_len > sector_size) {
        printf("Modify data crosses sector boundary.\n");
        printf("Please modify within one sector.\n");
        return -1;
    }

    sector = malloc(sector_size);

    if (!sector) {
        perror("malloc");
        return -1;
    }

    printf("\nModify operation\n");
    printf("-------------------------\n");
    printf("Address      : 0x%08x\n", address);
    printf("Sector start : 0x%08x\n", sector_start);
    printf("Sector size  : %u bytes\n", info->erasesize);

    /*
     * Step 1: Read complete sector
     */
    printf("\nReading complete sector...\n");

    if (lseek(fd, sector_start, SEEK_SET) < 0) {
        perror("lseek");
        free(sector);
        return -1;
    }

    if (read(fd, sector, sector_size) != (ssize_t)sector_size) {
        perror("read sector");
        free(sector);
        return -1;
    }

    /*
     * Step 2: Modify data in RAM
     */
    printf("Modifying data in RAM...\n");

    memcpy(sector + offset, new_data, new_len);

    /*
     * Step 3: Erase complete sector
     */
    printf("Erasing complete sector...\n");

    if (erase_sector(fd, sector_start, info) < 0) {
        free(sector);
        return -1;
    }

    /*
     * Step 4: Write complete sector
     */
    printf("Writing complete sector back...\n");

    if (lseek(fd, sector_start, SEEK_SET) < 0) {
        perror("lseek");
        free(sector);
        return -1;
    }

    if (write(fd, sector, sector_size) !=
        (ssize_t)sector_size) {

        perror("write sector");
        free(sector);
        return -1;
    }

    /*
     * Step 5: Verify modified data
     */
    printf("Verifying modified data...\n");

    if (lseek(fd, address, SEEK_SET) < 0) {
        perror("lseek");
        free(sector);
        return -1;
    }

    verify = malloc(new_len);

    if (!verify) {
        perror("malloc");
        free(sector);
        return -1;
    }

    if (read(fd, verify, new_len) !=
        (ssize_t)new_len) {

        perror("verify read");
        free(verify);
        free(sector);
        return -1;
    }

    for (i = 0; i < new_len; i++) {

        if (verify[i] != new_data[i]) {

            printf("Verification FAILED\n");

            free(verify);
            free(sector);

            return -1;
        }
    }

    free(verify);

    printf("Modify successful\n");

    printf("Modified data:\n");
    print_hex_text(new_data, new_len, address);

    free(sector);

    return 0;
}

static int handle_write(int fd, uint32_t address,
                        const uint8_t *data, size_t len,
                        struct mtd_info_user *info)
{
    int option;
    char confirm;
    uint32_t sector_start;

    sector_start = address -
                   (address % info->erasesize);

    printf("\nData will be written at 0x%08x\n", address);
    printf("Sector start : 0x%08x\n", sector_start);
    printf("Sector size  : %u bytes\n", info->erasesize);

    printf("\nChoose write method:\n");
    printf("1. Erase entire sector and write\n");
    printf("2. Modify sector (preserve other data)\n");
    printf("3. Cancel\n");

    printf("\nEnter option: ");

    if (scanf("%d", &option) != 1) {
        printf("Invalid option\n");
        return -1;
    }

    /*
     * Option 1:
     * Erase entire sector and write new data
     */
    if (option == 1) {

        printf("\nWARNING: Entire sector will be erased!\n");

        printf("Sector: 0x%08x - 0x%08x\n",
               sector_start,
               sector_start + info->erasesize - 1);

        printf("Continue? (y/n): ");

        if (scanf(" %c", &confirm) != 1) {
            printf("Invalid input\n");
            return -1;
        }

        if (confirm != 'y' && confirm != 'Y') {
            printf("Write cancelled\n");
            return 0;
        }

        if (erase_sector(fd, sector_start, info) < 0)
            return -1;

        if (write_data(fd, address, data, len) < 0)
            return -1;

        printf("Erase + write successful\n");
    }

    /*
     * Option 2:
     * Preserve other data in the sector
     */
    else if (option == 2) {

        if (modify_data(fd, address, data, len, info) < 0)
            return -1;

        printf("Modify operation successful\n");
    }

    /*
     * Option 3:
     * Cancel operation
     */
    else if (option == 3) {

        printf("Write cancelled\n");
    }

    else {

        printf("Invalid option\n");
        return -1;
    }

    return 0;
}

static void usage(const char *prog)
{
    printf("\nUsage:\n");

    printf("  %s <mtd> read <address> <length>\n", prog);
    printf("  %s <mtd> write <address> <data>\n", prog);
    printf("  %s <mtd> erase <address>\n", prog);

    printf("\nExamples:\n");

    printf("  %s /dev/mtd1 read 0x00000000 16\n", prog);
    printf("  %s /dev/mtd1 erase 0x00000000\n", prog);
    printf("  %s /dev/mtd1 write 0x00000000 \"SPI_NOR\"\n",
           prog);

    printf("\n");
}

int main(int argc, char *argv[])
{
    int fd;
    struct mtd_info_user info;
    uint32_t address;
    size_t len;

    if (argc < 4) {
        usage(argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDWR);

    if (fd < 0) {
        perror("open MTD");
        return 1;
    }

    printf("MTD device opened: %s\n", argv[1]);

    if (get_mtd_info(fd, &info) < 0) {
        close(fd);
        return 1;
    }

    address = strtoul(argv[3], NULL, 0);

    if (address >= info.size) {
        printf("Invalid address\n");
        close(fd);
        return 1;
    }

    /*
     * READ
     */
    if (!strcmp(argv[2], "read")) {

        if (argc != 5) {
            usage(argv[0]);
            close(fd);
            return 1;
        }

        len = strtoul(argv[4], NULL, 0);

        if (address + len > info.size) {
            printf("Read exceeds flash size\n");
            close(fd);
            return 1;
        }

        read_data(fd, address, len);
    }

    /*
     * ERASE
     */
    else if (!strcmp(argv[2], "erase")) {

        if (address % info.erasesize) {
            printf("Address must be aligned to erase size\n");
            close(fd);
            return 1;
        }

        erase_sector(fd, address, &info);
    }

    /*
     * WRITE
     */
    else if (!strcmp(argv[2], "write")) {

        if (argc != 5) {
            usage(argv[0]);
            close(fd);
            return 1;
        }

        len = strlen(argv[4]);

        if (address + len > info.size) {
            printf("Write exceeds flash size\n");
            close(fd);
            return 1;
        }

        if (handle_write(fd, address,
                         (uint8_t *)argv[4],
                         len, &info) < 0) {

            close(fd);
            return 1;
        }
    }

    /*
     * UNKNOWN COMMAND
     */
    else {

        printf("Unknown operation: %s\n", argv[2]);
        usage(argv[0]);

        close(fd);
        return 1;
    }

    close(fd);

    return 0;
}
