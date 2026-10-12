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

int count_words(char *str) {
    int count = 0, in_word = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (isspace((unsigned char)str[i])) {
            in_word = 0;
        } else if (!in_word) {
            count++;
            in_word = 1;
        }
    }

    return count;
}

int count_vowels(char *str) {
    int count = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        char ch = (char)tolower((unsigned char)str[i]);

        if (ch == 'a' || ch == 'e' || ch == 'i' ||
            ch == 'o' || ch == 'u') {
            count++;
        }
    }

    return count;
}

int main(void) {
    int server_fd, new_socket;
    int opt = 1;
    struct sockaddr_in address;
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

    new_socket = accept(server_fd, NULL, NULL);
    if (new_socket < 0) {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }

    printf("Client connected.\n");

    while (1) {
        size_t n = 0;
        char ch;

        /* Read one complete line */
        while (n < BUFFER_SIZE - 1) {
            ssize_t r = recv(new_socket, &ch, 1, 0);

            if (r <= 0)
                break;

            if (ch == '\n')
                break;

            buffer[n++] = ch;
        }

        if (n == 0)
            break;

        buffer[n] = '\0';

        if (strcmp(buffer, "bye") == 0)
            break;

        char *separator = strchr(buffer, '|');

        char response[BUFFER_SIZE];

        if (separator == NULL) {
            snprintf(response, sizeof(response),
                     "Invalid format. Use ANALYZE|TEXT");
        } else {
            *separator = '\0';

            char *command = buffer;
            char *text = separator + 1;

            if (strcmp(command, "ANALYZE") == 0) {
                int chars = (int)strlen(text);
                int words = count_words(text);
                int vowels = count_vowels(text);

                snprintf(response, sizeof(response),
                         "Chars=%d, Words=%d, Vowels=%d",
                         chars, words, vowels);
            } else {
                snprintf(response, sizeof(response),
                         "Invalid command");
            }
        }

        strcat(response, "\n");

        if (send(new_socket, response, strlen(response), 0) < 0)
            break;
    }

    close(new_socket);
    close(server_fd);

    printf("Server terminated.\n");
    return 0;
}
