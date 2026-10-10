
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define DATA_SIZE 4096

typedef struct {
    int student_id;
    char data[DATA_SIZE];
} Request;

void reap_children(int signal_number) {
    (void)signal_number;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
    }
}

void handle_client(int client_fd) {
    Request request;
    size_t received = 0;

    while (received < sizeof(request)) {
        ssize_t n = read(client_fd,
                         (char *)&request + received,
                         sizeof(request) - received);

        if (n == 0) {
            close(client_fd);
            _exit(0);
        }

        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("read");
            close(client_fd);
            _exit(1);
        }

        received += (size_t)n;
    }

    request.data[DATA_SIZE - 1] = '\0';

    char response[512];
    char *saveptr = NULL;
    char *line = strtok_r(request.data, "\n", &saveptr);
    int found = 0;

    while (line != NULL) {
        int id;
        char name[100], branch[100];
        char extra;

        if (sscanf(line, "%d %99s %99s %c",
                   &id, name, branch, &extra) == 3 &&
            id == request.student_id) {
            snprintf(response, sizeof(response),
                     "Record Found: %d %s %s", id, name, branch);
            found = 1;
            break;
        }

        line = strtok_r(NULL, "\n", &saveptr);
    }

    if (!found) {
        snprintf(response, sizeof(response), "Record Not Found");
    }

    size_t length = strlen(response);
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = write(client_fd, response + sent,
                          length - sent);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            break;
        }
        if (n == 0)
            break;
        sent += (size_t)n;
    }

    printf("Client PID %ld: %s\n", (long)getpid(), response);
    close(client_fd);
    _exit(0);
}

int main(void) {
    int server_fd;
    struct sockaddr_in address;

    signal(SIGPIPE, SIG_IGN);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = reap_children;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            if (errno == EINTR)
                continue;
            perror("accept");
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            close(client_fd);
        } else if (pid == 0) {
            close(server_fd);
            handle_client(client_fd);
        } else {
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}
