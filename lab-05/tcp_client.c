#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char input[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    const char *ip =
        (argc > 1) ? argv[1] : "127.0.0.1";

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) != 1) {
        printf("Invalid IP address\n");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Connected to server at %s\n", ip);

    while (1) {
        printf("\nEnter text to analyze or 'bye' to exit: ");

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        int quitting = strcmp(input, "bye") == 0;

        if (!quitting) {
            char request[BUFFER_SIZE];

            int written = snprintf(request, sizeof(request),
                                   "ANALYZE|%s\n", input);

            if (written < 0 || written >= BUFFER_SIZE) {
                printf("Input is too long\n");
                continue;
            }

            size_t total = (size_t)written;
            size_t sent = 0;

            while (sent < total) {
                ssize_t n = send(sock, request + sent,
                                 total - sent, 0);

                if (n <= 0) {
                    perror("Send failed");
                    close(sock);
                    return 1;
                }

                sent += (size_t)n;
            }
        } else {
            if (send(sock, "bye\n", 4, 0) < 0)
                perror("Send failed");

            break;
        }

        size_t n = 0;
        char ch;

        while (n < BUFFER_SIZE - 1) {
            ssize_t r = recv(sock, &ch, 1, 0);

            if (r <= 0)
                break;

            if (ch == '\n')
                break;

            buffer[n++] = ch;
        }

        buffer[n] = '\0';

        if (n == 0) {
            printf("Server disconnected\n");
            break;
        }

        printf("Server response: %s\n", buffer);
    }

    close(sock);
    printf("Client disconnected.\n");

    return 0;
}
