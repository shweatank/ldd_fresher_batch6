/*
 * secure_terminal_to_spi1.c
 *
 * User-space application that interfaces with the secure UART kernel driver
 * and the SPI display driver. Provides a simple terminal menu for encrypting
 * and decrypting text, which is then sent to the respective drivers for
 * processing and display.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

#include "../include/secure_comm_ioctl.h"// Include ioctl command definitions shared with the kernel driver

/*
 * Device paths:
 * - UART_DEV: custom kernel char driver
 * - SPI_DEV : display char driver
 * - USB_TTY : physical serial endpoint (Pi serial0)
 */
#define UART_DEV "/dev/secure_uart"
#define SPI_DEV  "/dev/ili9225_char"
#define USB_TTY  "/dev/serial0"

/* Prefix used to mark encrypted text */
#define ENC_PREFIX "ENC:"

/* Where 4-digit passkey is stored */
#define PASSKEY_FILE "/etc/secure_uart_passkey"

/* Local history of encrypted outputs (for decrypt validation) */
#define ENCRYPTED_DB_FILE "/tmp/secure_uart_encrypted.db"

/*
 * Configure tty in raw 115200 mode so bytes pass through
 * without line processing/echo translation.
 */
static int setup_tty(int fd)
{
    struct termios t;// Structure to hold terminal attributes
    if (tcgetattr(fd, &t) < 0)// Get current terminal attributes for the file descriptor fd and store them in t. If this fails, return -1.
        return -1;// Return error code for failure to get terminal attributes
    cfmakeraw(&t);// Configure the terminal attributes in t for raw mode, which disables line processing, echo, and other special handling of input/output characters.
    cfsetispeed(&t, B115200);// Set the input baud rate in t to 115200 baud. This configures the speed at which data is received from the serial line.
    cfsetospeed(&t, B115200);// Set the output baud rate in t to 115200 baud. This configures the speed at which data is sent to the serial line.
    t.c_cflag |= CLOCAL | CREAD;//  Set the control flags in t to enable the receiver (CREAD) and to ignore modem control lines (CLOCAL), which is common for serial communication where we don't want to be affected by carrier detect or other modem signals.
    return tcsetattr(fd, TCSANOW, &t);// Apply the modified terminal attributes in t to the file descriptor fd immediately (TCSANOW). If this fails, return -1; otherwise, return 0 for success.
}

/* Show menu and return selected mode (1..4), else -1 */
static int select_mode(void)
{
    int opt;// Variable to hold the user's menu selection
    printf("Select Mode: 1=Encrypt 2=Decrypt 3=Pass-through 4=Exit : ");// Prompt the user to select an operation mode from the menu
    if (scanf("%d", &opt) != 1)// Read an integer from standard input and store it in opt. If the input is not a valid integer, return -1.
        return -1;// Return error code for invalid input
    if (opt < 1 || opt > 4)// Check if the selected option is within the valid range of 1 to 4. If not, return -1.
        return -1;// Return error code for out-of-range selection
    return opt;// Return the valid selected mode (1, 2, 3, or 4)
}

/* Remove trailing '\n' from fgets input */
static void strip_newline(char *s)
{
    size_t n;// Variable to hold the length of the string s
    if (!s)// Check if the input string s is NULL. If it is, return immediately since there is nothing to strip.
        return;// Return early for NULL input
    n = strlen(s);  // Calculate the length of the string s and store it in n
    if (n > 0 && s[n - 1] == '\n')// Check if the last character of the string s is a newline character '\n'. If it is, replace it with a null terminator '\0' to remove the newline from the end of the string.
        s[n - 1] = '\0';// Remove the trailing newline character by replacing it with a null terminator
}

/* Validate passkey format: exactly 4 digits */
static bool is_four_digits(const char *s)
{
    int i;// Variable for iterating through the characters of the string s
    if (!s || strlen(s) != 4)// Check if the input string s is NULL or if its length is not equal to 4. If either condition is true, return false since a valid passkey must be exactly 4 characters long.
        return false;// Return false for NULL input or incorrect length
    for (i = 0; i < 4; i++) {// Iterate through each of the 4 characters in the string s
        if (s[i] < '0' || s[i] > '9')// Check if the current character s[i] is not a digit (i.e., not between '0' and '9'). If any character is not a digit, return false since a valid passkey must consist of only digits.
            return false;// Return false if any character is not a digit
    }
    return true;// If all characters are digits and the length is correct, return true to indicate that the passkey format is valid
}

/* Clear leftover chars from stdin (after scanf failures etc.) */
static void consume_stdin_line(void)
{
    int c;// Variable to hold characters read from standard input
    while ((c = getchar()) != '\n' && c != EOF) {}// Read characters from standard input one at a time until a newline character '\n' or end-of-file (EOF) is encountered. This is used to clear any leftover input from the input buffer, especially after a failed scanf that leaves invalid input in the buffer.
}

/* Load expected passkey from PASSKEY_FILE */
static int load_passkey(char *buf, size_t len)
{
    FILE *fp;// File pointer for reading the passkey file
    if (!buf || len < 5)// Check if the buffer for storing the passkey is NULL or if its length is less than 5 (4 digits plus null terminator). If either condition is true, return -1 since we need a valid buffer to store the passkey.
        return -1;// Return error code for invalid buffer
    fp = fopen(PASSKEY_FILE, "r");// Open the passkey file for reading. If the file cannot be opened (e.g., it does not exist or there are permission issues), return -1.
    if (!fp)//  Check if the file pointer fp is NULL, which indicates that the file could not be opened. If it is NULL, return -1 to indicate an error.
        return -1;// Return error code for failure to open passkey file
    if (!fgets(buf, len, fp)) {// Read a line from the passkey file into the buffer buf. If this fails (e.g., due to an I/O error), close the file and return -1.
        fclose(fp); //  Close the file before returning since we won't be using it anymore
        return -1;// Return error code for failure to read passkey
    }
    fclose(fp);// Close the file after successfully reading the passkey
    strip_newline(buf);// Remove any trailing newline character from the passkey that was read from the file
    if (!is_four_digits(buf))// Validate that the passkey read from the file is in the correct format (exactly 4 digits). If it is not valid, return -1.
        return -1;// Return error code for invalid passkey format
    return 0;// 
}

/* Append encrypted payload to local history file */
static void save_encrypted_payload(const char *payload)
{
    FILE *fp;// File pointer for writing to the encrypted payload history file
    if (!payload || !payload[0])// Check if the input payload is NULL or an empty string. If it is, return immediately since there is nothing to save.
        return;// Return early for invalid payload
    fp = fopen(ENCRYPTED_DB_FILE, "a");// Open the encrypted payload history file in append mode. If the file cannot be opened (e.g., due to permission issues), return without saving.
    if (!fp)// Check if the file pointer fp is NULL, which indicates that the file could not be opened. If it is NULL, return without saving the payload.
        return;// Return early for failure to open encrypted payload history file
    fprintf(fp, "%s\n", payload);// Write the payload followed by a newline character to the file. This appends the new encrypted payload to the history file.
    fclose(fp);
}

/* Check whether payload exists in encrypted history */
static bool payload_was_encrypted_before(const char *payload)
{
    char line[512];//   Buffer to hold lines read from the encrypted payload history file
    FILE *fp;// File pointer for reading the encrypted payload history file

    if (!payload || !payload[0])// Check if the input payload is NULL or an empty string. If it is, return false since an invalid payload cannot be in the history.
        return false;// Return false for invalid payload
    fp = fopen(ENCRYPTED_DB_FILE, "r");// Open the encrypted payload history file for reading. If the file cannot be opened (e.g., it does not exist), return false since we cannot verify the payload against the history.
    if (!fp)// Check if the file pointer fp is NULL, which indicates that the file could not be opened. If it is NULL, return false since we cannot verify the payload against the history.
        return false;// Return false for failure to open encrypted payload history file
    while (fgets(line, sizeof(line), fp)) {// Read lines from the encrypted payload history file one at a time into the buffer line
        strip_newline(line);// Remove any trailing newline character from the line read from the file
        if (line[0] == '\0')// If the line is empty after stripping the newline, skip it and continue to the next line
            continue;// Skip empty lines in the history file
         /* Check if the current line matches the input payload. If a match is found, close the file and return true to indicate that the payload was found in the history. */
        if (strcmp(line, payload) == 0) {// If the line matches the payload, we have found the payload in the history
            fclose(fp);// Close the file before returning since we have found the payload in the history
            return true;// Return true to indicate that the payload was found in the encrypted history
        }
    }
    fclose(fp);// Close the file after reading through the history
     /* If we have read through the entire file and did not find a match for the payload, return false to indicate that the payload was not found in the history. */    
    return false;
}

/*
 * Load the latest encrypted payload from ENCRYPTED_DB_FILE.
 * Returns 0 if found, -1 if not found/error.
 */
static int load_last_encrypted_payload(char *buf, size_t len)
{
    FILE *fp;// File pointer for reading the encrypted payload history file
    char line[512];// Buffer to hold lines read from the encrypted payload history file
    bool found = false;// Flag to indicate whether a valid encrypted payload was found in the history file

    if (!buf || len == 0)// Check if the input buffer is NULL or if its length is zero. If either condition is true, return -1 since we need a valid buffer to store the payload.
        return -1;

    fp = fopen(ENCRYPTED_DB_FILE, "r");// Open the encrypted payload history file for reading. If the file cannot be opened (e.g., it does not exist), return -1 since we cannot load any payload.
    if (!fp)// Check if the file pointer fp is NULL, which indicates that the file could not be opened. If it is NULL, return -1 to indicate an error.
        return -1;

    while (fgets(line, sizeof(line), fp)) {// Read lines from the encrypted payload history file one at a time into the buffer line
        size_t line_len;// Variable to hold the length of the current line read from the file
        strip_newline(line);// Remove any trailing newline character from the line read from the file
         /* Skip empty lines in the history file */
        if (line[0] == '\0')
            continue;
        line_len = strlen(line);// Calculate the length of the current line after stripping the newline character
         /* Copy the line to the output buffer, ensuring we do not overflow the buffer. If the line is longer than the buffer, truncate it. */  
        if (line_len > len - 1)
            line_len = len - 1;
        memcpy(buf, line, line_len);// Copy the line into the output buffer buf, up to line_len characters
        buf[line_len] = '\0';// Null-terminate the output buffer after copying the line
         /* Mark that we have found a valid encrypted payload in the history. We will return this last found payload after reading through the entire file. */
        found = true;
    }
    fclose(fp);
    return found ? 0 : -1;// Return 0 if we found at least one valid encrypted payload and loaded it into the buffer, or -1 if we did not find any valid payloads in the history
}

/*
 * End-to-end processing pipeline for one payload:
 * 1) send text to serial line
 * 2) try to read serial response (or fallback to input)
 * 3) send data to secure UART driver
 * 4) wait for processed data from driver
 * 5) display output via SPI driver
 */
static int process_and_display(int tty_fd, int uart_fd, int spi_fd, const char *input, int mode, char *output, size_t out_len)
{
    char tty_rx[256];// Buffer to hold data read from the serial line (or fallback to input if no response)
    ssize_t n;// Variable to hold the number of bytes read or written in various operations

    /* Step 1: send to physical serial endpoint */
    if (write(tty_fd, input, strlen(input)) < 0)
        perror("write ttyUSB0");

    /* Step 2: read from serial endpoint (with short timeout) */
    {
        struct pollfd tp = { .fd = tty_fd, .events = POLLIN };// Set up a pollfd structure to monitor the serial line file descriptor for incoming data (POLLIN event)
        int tr = poll(&tp, 1, 700);// Wait for up to 700 milliseconds for data to be available on the serial line. The return value tr indicates whether data is available (tr > 0), if a timeout occurred (tr == 0), or if an error occurred (tr < 0).
        if (tr > 0) {// If data is available on the serial line, read it into the tty_rx buffer. The number of bytes read is stored in n. If the read operation is successful and returns a positive number, null-terminate the string in tty_rx. If the read operation fails or returns 0, fall back to using the original input string as the data to process.
            n = read(tty_fd, tty_rx, sizeof(tty_rx) - 1);// Read data from the serial line into the tty_rx buffer, leaving space for a null terminator. The number of bytes read is stored in n.
            if (n > 0) {// If the read operation was successful and returned a positive number of bytes, null-terminate the string in tty_rx at the position n to ensure it is a valid C string.
                tty_rx[n] = '\0';// Null-terminate the string read from the serial line
            } else {// If the read operation failed or returned 0 bytes, fall back to using the original input string as the data to process. Copy the input string into tty_rx, ensuring that we do not overflow the buffer and that it is null-terminated.
                strncpy(tty_rx, input, sizeof(tty_rx) - 1);// Copy the original input string into tty_rx as a fallback, ensuring it is null-terminated
                tty_rx[sizeof(tty_rx) - 1] = '\0';// Null-terminate the fallback string in case it was truncated
            }// If the poll operation returned 0, it means a timeout occurred and no data was received on the serial line. In this case, we also fall back to using the original input string as the data to process.
        } else {
            strncpy(tty_rx, input, sizeof(tty_rx) - 1);// Copy the original input string into tty_rx as a fallback due to timeout or poll error, ensuring it is null-terminated
            tty_rx[sizeof(tty_rx) - 1] = '\0';// Null-terminate the fallback string in case it was truncated
             /* If the poll operation returned a negative value, it indicates an error occurred while waiting for data on the serial line. In this case, we print an error message but still fall back to using the original input string as the data to process. */
        }
    }

    /* Step 3: push data into secure kernel driver */
    n = write(uart_fd, tty_rx, strlen(tty_rx));// Write the data (either from the serial line or the original input) to the secure UART kernel driver. The number of bytes written is stored in n. If the write operation fails and returns a negative value, print an error message and return -1 to indicate failure.
    if (n < 0) {// Check if the write operation to the secure UART driver failed
        perror("write uart");// Print an error message indicating that the write operation to the secure UART driver failed
        return -1;// Return -1 to indicate failure in writing to the secure UART driver
    }

    /* Step 4: wait until driver has processed output */
    {
        struct pollfd pfd = {// Set up a pollfd structure to monitor the secure UART driver file descriptor for incoming data (POLLIN event), which indicates that the driver has processed the input and has output ready to be read.
            .fd = uart_fd,
            .events = POLLIN
        };
        int pr = poll(&pfd, 1, 3000);// Wait for up to 3000 milliseconds (3 seconds) for the secure UART driver to indicate that it has processed the input and has output ready. The return value pr indicates whether data is available (pr > 0), if a timeout occurred (pr == 0), or if an error occurred (pr < 0).
         /* If the poll operation returns a positive value, it means the secure UART driver has processed*/
        if (pr <= 0) {
            fprintf(stderr, "timeout waiting for processed UART data\n");
            return -1;
        }
    }

    n = read(uart_fd, output, out_len - 1);// Read the processed output from the secure UART driver into the output buffer. The number of bytes read is stored in n. If the read operation fails and returns a negative value, print an error message and return -1 to indicate failure. If the read operation is successful, null-terminate the output string.
    if (n < 0) {// Check if the read operation from the secure UART driver failed
        perror("read uart");// Print an error message indicating that the read operation from the secure UART driver failed
        return -1;
    }
    output[n] = '\0';// Null-terminate the output string read from the secure UART driver

     /* Print the processed output for debugging purposes before sending it to the SPI display driver. This allows us to see what data was processed by the secure UART driver before it is displayed. */

    printf("Processed data: %s\n", output);

    /* Save encrypted output for future decrypt verification */
    if (mode == SECURE_MODE_ENCRYPT)
        save_encrypted_payload(output);

    /* Step 5: send processed text to SPI display driver */
    if (write(spi_fd, output, strlen(output)) < 0)
        perror("write spi");
    else
        printf("Displayed on ILI9225 via /dev/ili9225_char\n");

    return 0;
}

int main(void)
{
    int uart_fd, spi_fd, tty_fd, mode;// File descriptors for the secure UART driver, SPI display driver, and serial line endpoint, as well as a variable to hold the selected mode of operation
    char input[256];// Buffer to hold user input text
    char active_payload[256];// Buffer to hold the current payload being processed, which may be modified with an ENC: prefix for encryption mode
    char passkey[16];// Buffer to hold the passkey entered by the user for decryption mode
    char expected_passkey[16];// Buffer to hold the expected passkey loaded from the PASSKEY_FILE for validation in decryption mode
    char output[256];// Buffer to hold the processed output received from the secure UART driver, which will be sent to the SPI display driver

    /* Open secure processing driver */
    uart_fd = open(UART_DEV, O_RDWR);
    if (uart_fd < 0) {
        perror("open secure_uart");
        return 1;
    }

    /* Open SPI display character device */
    spi_fd = open(SPI_DEV, O_WRONLY);
    if (spi_fd < 0) {
        perror("open ili9225_char");
        close(uart_fd);
        return 1;
    }

    /* Open serial line endpoint */
    tty_fd = open(USB_TTY, O_RDWR | O_NOCTTY | O_NONBLOCK);// Open the serial line endpoint (e.g., /dev/serial0) for reading and writing, without making it the controlling terminal (O_NOCTTY) and in non-blocking mode (O_NONBLOCK). If the open operation fails and returns a negative value, print an error message, close the previously opened secure UART and SPI driver file descriptors, and return 1 to indicate failure.
    if (tty_fd < 0) {
        perror("open /dev/ttyUSB0");
        close(spi_fd);
        close(uart_fd);
        return 1;
    }

    if (setup_tty(tty_fd) < 0)// Configure the serial line endpoint for raw mode and set the baud rate to 115200. If the setup operation fails and returns a negative value, print an error message, close all opened file descriptors, and return 1 to indicate failure.
        perror("setup tty");

    while (1) {
        mode = select_mode();// Show the menu and get the user's selected mode of operation. If the selection is invalid (e.g., not an integer or out of range), print an error message, consume any leftover input from standard input, and continue to show the menu again.

        if (mode < 0) {// Check if the selected mode is invalid
            fprintf(stderr, "invalid mode, try again\n");
            consume_stdin_line();// Clear any leftover input from standard input to prepare for the next menu selection
             /* Continue to the next iteration of the loop to show the menu again and prompt for a valid selection. */
            continue;
        }

        consume_stdin_line();// Clear any leftover input from standard input after a successful menu selection to prepare for the next user input (e.g., text or passkey) that will be read later in the loop.

        /* Exit option */
        if (mode == 4) {
            printf("Exiting application.\n");
            break;
        }

        /* Tell kernel driver which mode to use */
        if (ioctl(uart_fd, SECURE_UART_IOC_SET_MODE, &mode) < 0) {
            perror("ioctl set mode");
            continue;
        }

        if (mode == SECURE_MODE_DECRYPT) {
            /*
             * Decrypt flow:
             * - no new raw text accepted
             * - only latest known encrypted payload is allowed
             * - requires valid passkey
             */
            if (load_last_encrypted_payload(active_payload, sizeof(active_payload)) < 0) {// Attempt to load the latest encrypted payload from the history file into active_payload. If this operation fails (e.g., no history file or no valid payloads), print an error message indicating that no previously encrypted data was found and that the user should encrypt first, then continue to the next iteration of the loop to show the menu again.
                fprintf(stderr, "Error: no previously encrypted data found. Encrypt first.\n");// Print an error message indicating that no previously encrypted data was found and that the user should encrypt first
                 /* Continue to the next iteration of the loop to show the menu again and prompt for a valid selection. */
                continue;// Continue to the next iteration of the loop to show the menu again and prompt for a valid selection
            }

            if (!payload_was_encrypted_before(active_payload)) {// Check if the loaded active_payload exists in the encrypted history. If it does not exist, print an error message indicating that decryption is rejected due to invalid encrypted history, and continue to the next iteration of the loop to show the menu again.
                fprintf(stderr, "Error: decryption rejected due to invalid encrypted history.\n");
                continue;
            }

            if (load_passkey(expected_passkey, sizeof(expected_passkey)) < 0) {// Attempt to load the expected passkey from the PASSKEY_FILE into expected_passkey. If this operation fails (e.g., file does not exist, cannot be read, or invalid format), print an error message indicating that the passkey could not be loaded and provide instructions for creating the file with a valid 4-digit passkey, then continue to the next iteration of the loop to show the menu again.
                fprintf(stderr, "Unable to load 4-digit passkey from %s\n", PASSKEY_FILE);
                fprintf(stderr, "Create file with exactly 4 digits, e.g.: echo 1234 | sudo tee %s\n", PASSKEY_FILE);
                continue;
            }

            printf("Decrypting latest encrypted payload: %s\n", active_payload);
            printf("Enter 4-digit passkey: ");

            if (!fgets(passkey, sizeof(passkey), stdin)) {// Read a line of input from standard input into the passkey buffer. If this operation fails (e.g., due to an I/O error), print an error message indicating that the passkey read failed, and continue to the next iteration of the loop to show the menu again.
                fprintf(stderr, "passkey read failed\n");// Print an error message indicating that the passkey read failed
                 /* Continue to the next iteration of the loop to show the menu again and prompt for a valid selection. */  
                continue;
            }

            strip_newline(passkey);// Remove any trailing newline character from the passkey that was read from standard input

             /* Validate the entered passkey against the expected passkey loaded from the file. If the entered passkey is not in the correct format (not exactly 4 digits) or does not match the expected passkey, print an error message indicating that the passkey is incorrect and that decryption is denied, then continue to the next iteration of the loop to show the menu again. */        

            if (!is_four_digits(passkey) || strcmp(passkey, expected_passkey) != 0) {
                fprintf(stderr, "Incorrect passkey. Decryption denied.\n");
                continue;
            }

        } else {
            /*
             * Encrypt / Pass flow:
             * - ask user text
             * - for encrypt, add ENC: prefix marker
             */
            printf("Enter text: ");
            if (!fgets(input, sizeof(input), stdin)) {// Read a line of input from standard input into the input buffer. If this operation fails (e.g., due to an I/O error), print an error message indicating that the input read failed, and continue to the next iteration of the loop to show the menu again.
                fprintf(stderr, "input read failed\n");
                continue;
            }

            strip_newline(input);// Remove any trailing newline character from the input that was read from standard input

             /* Copy the user input into active_payload, ensuring that it is null-terminated. This will be the payload that we process and send to the secure UART driver. If the mode is SECURE_MODE_ENCRYPT, we will add an ENC: prefix to this payload before processing. */     
            strncpy(active_payload, input, sizeof(active_payload) - 1);
            active_payload[sizeof(active_payload) - 1] = '\0';
/* If the selected mode is SECURE_MODE_ENCRYPT, we add an ENC: prefix to the active_payload to mark it as encrypted text. We need to ensure that we do not overflow the active_payload buffer when adding the prefix, so we calculate the length of the input and adjust it if necessary before adding the prefix. */
            if (mode == SECURE_MODE_ENCRYPT) {
                size_t prefix_len = strlen(ENC_PREFIX);
                size_t input_len = strlen(active_payload);

                if (input_len > sizeof(active_payload) - prefix_len - 1)
                    input_len = sizeof(active_payload) - prefix_len - 1;

                memmove(active_payload + prefix_len, active_payload, input_len);
                memcpy(active_payload, ENC_PREFIX, prefix_len);
                active_payload[prefix_len + input_len] = '\0';
            }
        }

        if (process_and_display(tty_fd, uart_fd, spi_fd, active_payload, mode, output, sizeof(output)) < 0)// Call the process_and_display function to handle the end-to-end processing of the active_payload based on the selected mode. If this function returns a negative value, it indicates that an error occurred during processing, so we print an error message indicating that the operation failed and that we are returning to mode selection, then continue to the next iteration of the loop to show the menu again.
            fprintf(stderr, "Operation failed, returning to mode selection.\n");
    }

    close(tty_fd);
    close(spi_fd);
    close(uart_fd);
    return 0;
}
