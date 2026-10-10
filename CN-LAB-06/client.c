#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define DATA_SIZE 4096

typedef struct {
    int student_id;
    char data[DATA_SIZE];
} Request;

int main(void) {
    FILE *file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("data.txt");
        return 1;
    }

    Request request;
    memset(&request, 0, sizeof(request));

    size_t count = fread(request.data, 1,
                         DATA_SIZE - 1, file);

    if (ferror(file)) {
        perror("fread");
        fclose(file);
        return 1;
    }

    if (count == DATA_SIZE - 1 && !feof(file)) {
        fprintf(stderr, "data.txt is too large.\n");
        fclose(file);
        return 1;
    }

    fclose(file);

    printf("Enter Student ID to search: ");
    if (scanf("%d", &request.student_id) != 1) {
        fprintf(stderr, "Invalid Student ID.\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    size_t sent = 0;
    while (sent < sizeof(request)) {
        ssize_t n = write(sock, (char *)&request + sent,
                          sizeof(request) - sent);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("write");
            close(sock);
            return 1;
        }
        if (n == 0) {
            fprintf(stderr, "Connection closed while sending.\n");
            close(sock);
            return 1;
        }
        sent += (size_t)n;
    }

    char response[512];
    size_t received = 0;

    while (received < sizeof(response) - 1) {
        ssize_t n = read(sock, response + received,
                         sizeof(response) - 1 - received);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("read");
            close(sock);
            return 1;
        }
        if (n == 0)
            break;
        received += (size_t)n;
    }

    response[received] = '\0';
    printf("Server Response: %s\n",
           received ? response : "No response");

    close(sock);
    return received ? 0 : 1;
}
