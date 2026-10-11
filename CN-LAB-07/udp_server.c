// Roll: 24625 Service Type: B d = 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 30625
#define BUFFER_SIZE 1024
#define ROLL "24625"

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    printf("Server running on port %d\n", PORT);

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&client_addr, &client_len);

        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[n] = '\0';

        printf("\nClient: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        char expected_prefix[50];
        snprintf(expected_prefix, sizeof(expected_prefix), "%s#", ROLL);

        if (strncmp(buffer, expected_prefix,
                    strlen(expected_prefix)) != 0) {

            char *error = "ERROR: Unauthorized client";

            sendto(sockfd, error, strlen(error), 0,
                   (struct sockaddr *)&client_addr, client_len);

            printf("Sent response: %s\n", error);
            continue;
        }

        char *request = buffer + strlen(expected_prefix);

        printf("Received message: %s\n", request);

        if (strcmp(request, "EXIT") == 0) {
            char *reply = "Server exiting";

            sendto(sockfd, reply, strlen(reply), 0,
                   (struct sockaddr *)&client_addr, client_len);

            printf("Sent response: %s\n", reply);
            break;
        }

        char temp[BUFFER_SIZE];
        strcpy(temp, request);

        char *op = strtok(temp, "|");
        char *count_str = strtok(NULL, "|");
        char *numbers = strtok(NULL, "|");

        char response[BUFFER_SIZE];

        if (op == NULL || count_str == NULL || numbers == NULL) {
            strcpy(response, "ERROR: Invalid request");
        } else {
            int count = atoi(count_str);
            int values[100];
            int actual = 0;

            char *token = strtok(numbers, " ");

            while (token != NULL && actual < count && actual < 100) {
                values[actual++] = atoi(token);
                token = strtok(NULL, " ");
            }

            if (count <= 0 || actual != count) {
                strcpy(response, "ERROR: Invalid integer list");
            } else {
                long sum = 0;
                int min = values[0];
                int max = values[0];

                for (int i = 0; i < count; i++) {
                    sum += values[i];

                    if (values[i] < min)
                        min = values[i];

                    if (values[i] > max)
                        max = values[i];
                }

                if (strcmp(op, "SUM") == 0) {
                    snprintf(response, sizeof(response), "%ld", sum);
                }
                else if (strcmp(op, "MAX") == 0) {
                    snprintf(response, sizeof(response), "%d", max);
                }
                else if (strcmp(op, "MIN") == 0) {
                    snprintf(response, sizeof(response), "%d", min);
                }
                else if (strcmp(op, "AVG") == 0) {
                    double avg = (double)sum / count;
                    snprintf(response, sizeof(response), "%.2f", avg);
                }
                else {
                    strcpy(response, "ERROR: Invalid operation");
                }
            }
        }

        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, client_len);

        printf("Sent response: %s\n", response);
    }

    close(sockfd);
    return 0;
}
