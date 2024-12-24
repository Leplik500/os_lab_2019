#include "socket_utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>

struct Server {
    char ip[255];
    int port;
};

struct Task {
    struct Server server;
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
    char *end = NULL;
    unsigned long long i = strtoull(str, &end, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "Out of uint64_t range: %s\n", str);
        return false;
    }
    if (errno != 0)
        return false;

    *val = i;
    return true;
}

void *connectToServer(void *arg) {
    struct Task *task = (struct Task *)arg;

    int sck = connect_to_server(task->server.ip, task->server.port);

    uint64_t begin = htonll(task->begin);
    uint64_t end = htonll(task->end);
    uint64_t mod = htonll(task->mod);

    char task_data[sizeof(uint64_t) * 3];
    memcpy(task_data, &begin, sizeof(uint64_t));
    memcpy(task_data + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task_data + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send_data(sck, task_data, sizeof(task_data)) == -1) {
        perror("Send failed");
        close(sck);
        return NULL;
    }

    char response[sizeof(uint64_t)];
    if (receive_data(sck, response, sizeof(response)) == -1) {
        perror("Receive failed");
        close(sck);
        return NULL;
    }

    uint64_t answer = ntohll(*((uint64_t *)response));

    close(sck);

    printf("Received answer: %" PRIu64 " from server %s:%d\n", answer, task->server.ip, task->server.port);
    return (void *)(uintptr_t)answer;
}

int main(int argc, char **argv) {
    uint64_t k = -1;
    uint64_t mod = -1;

    struct Server servers[10];
    int servers_num = 0;

    while (true) {
        static struct option options[] = {{"k", required_argument, 0, 0},
                                           {"mod", required_argument, 0, 0},
                                           {"servers", required_argument, 0, 0},
                                           {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        ConvertStringToUI64(optarg, &k);
                        break;
                    case 1:
                        ConvertStringToUI64(optarg, &mod);
                        break;
                    case 2:
                        {
                            FILE *file = fopen(optarg, "r");
                            if (!file) {
                                perror("Failed to open servers list file");
                                return EXIT_FAILURE;
                            }
                            while (fscanf(file, "%s %d", servers[servers_num].ip, &servers[servers_num].port) == 2 && servers_num < 10) {
                                servers_num++;
                            }
                            fclose(file);
                        }
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

    if (k == -1 || mod == -1 || servers_num == 0) {
        fprintf(stderr, "Usage: %s --k <factorial> --mod <modulus> --servers /path/to/file\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct Task tasks[servers_num];
    uint64_t chunk_size = k / servers_num;
    uint64_t remainder = k % servers_num;
    uint64_t begin = 1;

    for (int i = 0; i < servers_num; i++) {
        tasks[i].server = servers[i];
        tasks[i].begin = begin;
        tasks[i].end = begin + chunk_size - 1;
        if (i < remainder)
            tasks[i].end++;
        tasks[i].mod = mod;
        begin = tasks[i].end + 1;
    }

    pthread_t threads[servers_num];
    uint64_t total_result = 1;

    for (int i = 0; i < servers_num; i++) {
        if (pthread_create(&threads[i], NULL, connectToServer, (void *)&tasks[i]) != 0) {
            perror("Failed to create thread");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < servers_num; i++) {
        void *result_ptr;
        if (pthread_join(threads[i], &result_ptr) != 0) {
            perror("Failed to join thread");
            return EXIT_FAILURE;
        }
        uint64_t answer = (uint64_t)(uintptr_t)result_ptr;
        total_result = (total_result * answer) % mod;
    }

    printf("Total result from all servers: %" PRIu64 "\n", total_result);
    return EXIT_SUCCESS;
}