#include "socket_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <errno.h>

// Function to check endianness
int is_big_endian() {
    uint64_t x = 1;
    return !*(char *)&x;
}

// Function to convert uint64_t to network byte order
uint64_t htonll(uint64_t value) {
    if (is_big_endian())
        return value;
    else
        return ((value << 56) & 0xff00000000000000ULL) |
               ((value << 40) & 0x00ff000000000000ULL) |
               ((value << 24) & 0x0000ff0000000000ULL) |
               ((value << 8)  & 0x000000ff00000000ULL) |
               ((value >> 8)  & 0x00000000ff000000ULL) |
               ((value >> 24) & 0x0000000000ff0000ULL) |
               ((value >> 40) & 0x000000000000ff00ULL) |
               ((value >> 56) & 0x00000000000000ffULL);
}

// Function to convert uint64_t from network byte order to host byte order
uint64_t ntohll(uint64_t value) {
    return htonll(value);
}

int create_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int bind_socket(int sockfd, int port) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return 0;
}

int listen_socket(int sockfd) {
    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return 0;
}

int accept_connection(int sockfd, struct sockaddr_in *client_addr) {
    socklen_t addr_len = sizeof(*client_addr);
    int client_fd = accept(sockfd, (struct sockaddr *)client_addr, &addr_len);
    if (client_fd < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    return client_fd;
}

int connect_to_server(const char *ip, int port) {
    int sockfd = create_socket();

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

ssize_t send_data(int sockfd, const void *data, size_t size) {
    const char *buf = (const char *)data;
    ssize_t total_sent = 0;
    while (total_sent < size) {
        ssize_t sent = send(sockfd, buf + total_sent, size - total_sent, 0);
        if (sent <= 0) {
            if (sent == -1 && errno == EINTR)
                continue;
            return -1;
        }
        total_sent += sent;
    }
    return total_sent;
}

ssize_t receive_data(int sockfd, void *buffer, size_t size) {
    char *buf = (char *)buffer;
    ssize_t total_received = 0;
    while (total_received < size) {
        ssize_t received = recv(sockfd, buf + total_received, size - total_received, 0);
        if (received <= 0) {
            if (received == -1 && errno == EINTR)
                continue;
            return -1;
        }
        total_received += received;
    }
    return total_received;
}