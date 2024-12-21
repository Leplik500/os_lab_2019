#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <stdint.h>
#include <netinet/in.h>

int create_socket();
int bind_socket(int sockfd, int port);
int listen_socket(int sockfd);
int accept_connection(int sockfd, struct sockaddr_in *client_addr);
int connect_to_server(const char *ip, int port);
ssize_t send_data(int sockfd, const void *data, size_t size);
ssize_t receive_data(int sockfd, void *buffer, size_t size);

#endif // SOCKET_UTILS_H
