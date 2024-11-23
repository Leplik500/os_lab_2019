#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <getopt.h>
#include "find_min_max.h"
#include "utils.h"

#define BUFFER_SIZE 256

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;
    int timeout = -1; // Initialize timeout variable

    while (true) {
        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {"timeout", required_argument, 0, 't'}, // Add timeout option
            {0, 0, 0, 0}
        };
        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0: seed = atoi(optarg); break;
                    case 1: array_size = atoi(optarg); break;
                    case 2: pnum = atoi(optarg); break;
                    case 3: with_files = true; break;
                    case 4: timeout = atoi(optarg); break; // Parse timeout
                }
                break;
            case 'f': with_files = true; break;
            case 't': timeout = atoi(optarg); break; // Handle command-line argument for timeout
            case '?': break;
            default: printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"]\n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);
    
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int pipes[pnum][2];
    for (int i = 0; i < pnum; i++) {
        if (!with_files && pipe(pipes[i]) == -1) {
            perror("pipe");
            return 1;
        }
        
        pid_t child_pid = fork();
        if (child_pid >= 0) {
            if (child_pid == 0) { // Child process
                // Child process logic here...
                return 0; // Exit child process
            } else { // Parent process
                if (!with_files) close(pipes[i][1]); // Close writing end in parent
            }
        } else {
            printf("Fork failed!\n");
            return 1;
        }
    }

    if (timeout > 0) {
        sleep(timeout);
        for (int i = 0; i < pnum; i++) {
            kill(-getpid(), SIGKILL);
        }
    }

    // while (wait(NULL) > 0);

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;
    
    free(array);
    
    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    
    return 0;
}
