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
    struct sockaddr_in server_addr;
    char input[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    const char *ip = (argc > 1) ? argv[1] : "127.0.0.1";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) != 1) {
        printf("Invalid IP address\n");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("Connected to server at %s.\n", ip);

    while (1) {
        printf("\nEnter command (UP|text, LOW|text, REV|text or bye): ");

        if (fgets(input, sizeof(input), stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';

        size_t len = strlen(input);
        int quitting = (strcmp(input, "bye") == 0);

        if (send(sock, input, len, 0) < 0 ||
            send(sock, "\n", 1, 0) < 0) {
            perror("Send failed");
            break;
        }

        /* Read one complete response line */
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

        if (n > 0)
            printf("Server response: %s\n", buffer);

        if (quitting || n == 0)
            break;
    }

    close(sock);
    printf("Client disconnected.\n");
    return 0;
}
