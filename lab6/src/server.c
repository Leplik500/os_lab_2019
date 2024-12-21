#include "socket_utils.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>
#include <unistd.h>

void handleClient(int client_fd) {
    // Implementation of handling client requests
    char buffer[1024];
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    
    if (bytes_received > 0) {
        // Process data received from client
        send(client_fd, buffer, bytes_received, 0); // Echo back for example
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
        
        handleClient(client_fd); // Call the function here
    }

    close(server_fd);

    return EXIT_SUCCESS;
}
