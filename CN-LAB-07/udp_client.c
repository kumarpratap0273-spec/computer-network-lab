// Roll: 24625 Service Type: B d = 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s <server_ip> <roll>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    char *roll = argv[2];

    int last3 = atoi(roll + strlen(roll) - 3);
    int port = 30000 + last3;

    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);

    char input[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip,
                  &server_addr.sin_addr) <= 0) {
        printf("Invalid server IP\n");
        close(sockfd);
        return 1;
    }

    printf("Connected to UDP server port %d\n", port);

    while (1) {

        printf("Enter command: ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            strcpy(input, "EXIT");
        }

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "EXIT") == 0) {
            snprintf(message, sizeof(message),
                     "%s#EXIT", roll);
        } else {
            snprintf(message, sizeof(message),
                     "%s#%s", roll, input);
        }

        sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&server_addr,
               sizeof(server_addr));

        memset(response, 0, sizeof(response));

        int n = recvfrom(sockfd, response,
                         BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&server_addr,
                         &server_len);

        if (n < 0) {
            perror("recvfrom");
            break;
        }

        response[n] = '\0';

        printf("Server Response: %s\n", response);

        if (strcmp(input, "EXIT") == 0)
            break;
    }

    close(sockfd);

    return 0;
}
