#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <stdint.h>
#include <arpa/inet.h>

// Function prototypes
int create_socket();
int bind_socket(int sockfd, int port);
int listen_socket(int sockfd);
int accept_connection(int sockfd, struct sockaddr_in *client_addr);
int connect_to_server(const char *ip, int port);
ssize_t send_data(int sockfd, const void *data, size_t size);
ssize_t receive_data(int sockfd, void *buffer, size_t size);

// Endianness conversion functions
uint64_t htonll(uint64_t value);
uint64_t ntohll(uint64_t value);

#endif // SOCKET_UTILS_H