#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#define SADDR struct sockaddr

int main(int argc, char *argv[]) {
  int fd;
  int nread;
  char *buf;
  struct addrinfo hints, *res, *p;

  if (argc < 4) {
    printf("Usage: %s <hostname> <port> <bufsize>\n", argv[0]);
    exit(1);
  }

  char port_str[10];
  snprintf(port_str, sizeof(port_str), "%s", argv[2]);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;  // TCP

  if (getaddrinfo(argv[1], port_str, &hints, &res) != 0) {
    perror("getaddrinfo");
    exit(1);
  }

  fd = -1;
  for (p = res; p != NULL; p = p->ai_next) {
    if ((fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      perror("socket");
      continue;
    }

    if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) {
      break;  // Connection successful
    } else {
      perror("connect");
      close(fd);
      fd = -1;
    }
  }

  if (p == NULL) {
    fprintf(stderr, "Could not connect\n");
    exit(1);
  }

  freeaddrinfo(res);

  int bufsize = atoi(argv[3]);
  buf = malloc(bufsize);
  if (buf == NULL) {
    perror("malloc");
    exit(1);
  }

  write(1, "Input message to send\n", 22);

  while ((nread = read(0, buf, bufsize)) > 0) {
    if (write(fd, buf, nread) < 0) {
      perror("write");
      exit(1);
    }
  }

  free(buf);
  close(fd);
  exit(0);
}