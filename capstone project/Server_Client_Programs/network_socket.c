#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

/*
 * Create and initialize a TCP server socket
 * Binds to the given port and starts listening for incoming connections
 */
int start_server(int port)
{
    // Create TCP socket (IPv4, stream-based)
    int s = socket(AF_INET, SOCK_STREAM, 0); // AF_INET = IPv4
    if (s < 0)
    {
        printf("socket error\n");
        return -1;
    }

    // Allow reuse of address to avoid "Address already in use" errors
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configure server address structure
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;         // IPv4
    addr.sin_port = htons(port);       // Convert port to network byte order
    addr.sin_addr.s_addr = INADDR_ANY; // Accept connections on all interfaces

    // Bind socket to specified IP/port
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("bind error\n");
        close(s);
        return -1;
    }

    // Start listening for incoming connections
    if (listen(s, 20) < 0)
    {
        printf("listen error\n");
        close(s);
        return -1;
    }

    // Return listening socket descriptor
    return s;
}

/*
 * Connect to a remote TCP server (client-side socket)
 * Returns connected socket descriptor or -1 on failure
 */
int connect_peer(const char *ip, int port)
{
    // Create TCP socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        return -1;

    // Configure remote server address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;   // IPv4
    addr.sin_port = htons(port); // Convert port to network byte order

    // Convert IP string to binary format
    inet_pton(AF_INET, ip, &addr.sin_addr);

    // Attempt connection to server
    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(s);
        return -1;
    }

    // Return connected socket descriptor
    return s;
}
