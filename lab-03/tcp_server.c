#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

void to_upper(char *str) {
    for (int i = 0; str[i]; i++)
        str[i] = (char)toupper((unsigned char)str[i]);
}

void to_lower(char *str) {
    for (int i = 0; str[i]; i++)
        str[i] = (char)tolower((unsigned char)str[i]);
}

void reverse(char *str) {
    int i = 0, j = (int)strlen(str) - 1;
    while (i < j) {
        char temp = str[i];
        str[i++] = str[j];
        str[j--] = temp;
    }
}

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }

    printf("Client connected.\n");

    while (1) {
        size_t n = 0;
        char ch;

        /* Read one complete line from the client */
        while (n < BUFFER_SIZE - 1) {
            ssize_t r = recv(client_fd, &ch, 1, 0);

            if (r <= 0)
                break;

            if (ch == '\n')
                break;

            buffer[n++] = ch;
        }

        if (n == 0)
            break;

        buffer[n] = '\0';

        if (strcmp(buffer, "bye") == 0) {
            const char *reply = "Goodbye\n";
            send(client_fd, reply, strlen(reply), 0);
            break;
        }

        char *separator = strchr(buffer, '|');

        if (separator == NULL) {
            strcpy(buffer, "Invalid format. Use COMMAND|TEXT");
        } else {
            *separator = '\0';
            char *command = buffer;
            char *text = separator + 1;

            if (strcmp(command, "UP") == 0) {
                to_upper(text);
            } else if (strcmp(command, "LOW") == 0) {
                to_lower(text);
            } else if (strcmp(command, "REV") == 0) {
                reverse(text);
            } else {
                strcpy(text, "Invalid command");
            }

            memmove(buffer, text, strlen(text) + 1);
        }

        strcat(buffer, "\n");
        send(client_fd, buffer, strlen(buffer), 0);
    }

    close(client_fd);
    close(server_fd);

    printf("Server terminated.\n");
    return 0;
}
