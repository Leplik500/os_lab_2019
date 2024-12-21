#include "socket_utils.h" 
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <errno.h>      
#include <unistd.h>
#include <getopt.h> 

struct Server {
    char ip[255];
    int port;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
    uint64_t result = 0;
    a = a % mod;
    while (b > 0) {
        if (b % 2 == 1)
            result = (result + a) % mod;
        a = (a * 2) % mod;
        b /= 2;
    }
    return result % mod;
}

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
    struct Server *server = (struct Server *)arg;

    int sck = connect_to_server(server->ip, server->port); 

    uint64_t begin = 1; 
    uint64_t end = 100; 
    uint64_t mod = 1000;

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    send_data(sck, task, sizeof(task));

    char response[sizeof(uint64_t)];
    receive_data(sck, response, sizeof(response)); 

    uint64_t answer = 0;
    memcpy(&answer, response, sizeof(uint64_t));
    
    printf("Result from server %s:%d: %" PRIu64 "\n", server->ip, server->port, answer); // Use PRIu64

    close(sck);    
}

int main(int argc, char **argv) {
    uint64_t k = -1;
    uint64_t mod = -1;

    struct Server servers[10]; 
    int servers_num = 0;

    while (true) {
        int current_optind = optind ? optind : 1;

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
                            while (fscanf(file, "%s %d", servers[servers_num].ip,
                                          &servers[servers_num].port) != EOF && servers_num < 10) {
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
        fprintf(stderr,
                "Usage: %s --k <factorial> --mod <modulus> --servers /path/to/file\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    pthread_t threads[servers_num];

    for (int i = 0; i < servers_num; i++) {
        pthread_create(&threads[i], NULL, connectToServer,
                       (void *)&servers[i]);
    }

    for (int i = 0; i < servers_num; i++) {
        pthread_join(threads[i], NULL);
    }

    return EXIT_SUCCESS;
}
