#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5002
#define SERVER_IP "10.116.11.19"

int main()
{
    int sockfd;
    struct sockaddr_in server;
    char buffer[1024];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    connect(sockfd, (struct sockaddr *)&server, sizeof(server));

    printf("Connected to server\n");

    while (1)
    {
        int n = read(sockfd, buffer, sizeof(buffer));

        if (n > 0)
        {
            printf("Minicom Data: %.*s\n", n, buffer);
        }
    }

    close(sockfd);
    return 0;
}
