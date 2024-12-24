#include "socket_utils.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>
#include <unistd.h>
#include <inttypes.h>

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
    uint64_t result = 1;
    for (uint64_t i = a; i <= b; i++) {
        result = (result * (i % mod)) % mod;
    }
    return result;
}

void handleClient(int client_fd) {
    char buffer[1024];
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

    if (bytes_received != sizeof(uint64_t) * 3) {
        perror("Incorrect data received");
        close(client_fd);
        return;
    }

    uint64_t begin, end, mod;
    memcpy(&begin, buffer, sizeof(uint64_t));
    memcpy(&end, buffer + sizeof(uint64_t), sizeof(uint64_t));
    memcpy(&mod, buffer + 2 * sizeof(uint64_t), sizeof(uint64_t));

    begin = ntohll(begin);
    end = ntohll(end);
    mod = ntohll(mod);

    printf("Received begin: %" PRIu64 ", end: %" PRIu64 ", mod: %" PRIu64 "\n", begin, end, mod);

    uint64_t result = MultModulo(begin, end, mod);

    printf("Sending result: %" PRIu64 "\n", result);

    result = htonll(result);

    size_t bytes_sent = 0;
    while (bytes_sent < sizeof(result)) {
        ssize_t sent = send(client_fd, (char *)&result + bytes_sent, sizeof(result) - bytes_sent, 0);
        if (sent < 0) {
            perror("Send failed");
            close(client_fd);
            return;
        }
        bytes_sent += sent;
    }

    close(client_fd);
}

int main(int argc, char **argv) {
    int port = 20001;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"port", required_argument, 0, 0},
                                           {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        port = atoi(optarg);
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case '?':
                printf("Arguments error\n");
                break;
            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    int server_fd = create_socket();
    bind_socket(server_fd, port);
    listen_socket(server_fd);

    printf("Server listening at port %d\n", port);

    while (true) {
        struct sockaddr_in client_addr;
        int client_fd = accept_connection(server_fd, &client_addr);

        handleClient(client_fd);
    }

    close(server_fd);

    return EXIT_SUCCESS;
}