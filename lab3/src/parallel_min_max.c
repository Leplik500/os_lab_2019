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

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

#define BUFFER_SIZE 256

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"seed", required_argument, 0, 0},
                                           {"array_size", required_argument, 0, 0},
                                           {"pnum", required_argument, 0, 0},
                                           {"by_files", no_argument, 0, 'f'},
                                           {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        if (seed < 0) {
                            fprintf(stderr, "Error: Seed must be a non-negative integer.\n");
                            return 1;
                        }
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        if (array_size <= 0) {
                            fprintf(stderr, "Error: Array size must be a positive integer.\n");
                            return 1;
                        }
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        if (pnum <= 0 || pnum % 2 != 0) { // Ensure even number of processes
                            fprintf(stderr, "Error: pnum must be a positive even integer.\n");
                            return 1;
                        }
                        break;
                    case 3:
                        with_files = true;
                        break;

                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case 'f':
                with_files = true;
                break;

            case '?':
                break;

            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (optind < argc) {
        printf("Has at least one no option argument\n");
        return 1;
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
               argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);
    int active_child_processes = 0;

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int pipes[pnum][2]; // Create pipes for each child process

    for (int i = 0; i < pnum; i++) {
        if (with_files && i % 2 == 0) {
            // Create files for min and max results
            char filename[BUFFER_SIZE];
            snprintf(filename, sizeof(filename), "result_%d.txt", i / 2);
            FILE *file = fopen(filename, "w");
            if (!file) {
                perror("Failed to open file");
                return 1;
            }
            fclose(file); // Close file immediately after creation
        } else if (!with_files) {
            // Create pipes
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                return 1;
            }
        }

        pid_t child_pid = fork();
        if (child_pid >= 0) {
            // successful fork
            active_child_processes += 1;

            if (child_pid == 0) { // Child process
                if (with_files) {
                    // File handling logic
                    char filename[BUFFER_SIZE];
                    snprintf(filename, sizeof(filename), "result_%d.txt", i / 2); // Use the same file for min/max
                    FILE *file = fopen(filename, "a"); // Append mode
                    if (!file) {
                        perror("Failed to open file");
                        exit(1);
                    }

                    // Calculate min or max for a portion of the array
                    int start_index = i * (array_size / pnum);
                    int end_index = (i + 1) * (array_size / pnum);
                    if (i == pnum - 1) end_index = array_size; // Last child takes the remainder

                    if (i % 2 == 0) { // Even indexed processes find min
                        int min = INT_MAX;
                        for (int j = start_index; j < end_index; j++) {
                            if (array[j] < min) min = array[j];
                        }
                        fprintf(file, "%d\n", min); // Write min to file
                    } else { // Odd indexed processes find max
                        int max = INT_MIN;
                        for (int j = start_index; j < end_index; j++) {
                            if (array[j] > max) max = array[j];
                        }
                        fprintf(file, "%d\n", max); // Write max to file
                    }

                    fclose(file); // Close the file
                } else { // Pipe handling logic
                    close(pipes[i][0]); // Close reading end in child

                    // Calculate min or max for a portion of the array
                    int start_index = i * (array_size / pnum);
                    int end_index = (i + 1) * (array_size / pnum);
                    if (i == pnum - 1) end_index = array_size; // Last child takes the remainder

                    if (i % 2 == 0) { // Even indexed processes find min
                        int min = INT_MAX;
                        for (int j = start_index; j < end_index; j++) {
                            if (array[j] < min) min = array[j];
                        }
                        write(pipes[i][1], &min, sizeof(int)); // Write min to pipe
                    } else { // Odd indexed processes find max
                        int max = INT_MIN;
                        for (int j = start_index; j < end_index; j++) {
                            if (array[j] > max) max = array[j];
                        }
                        write(pipes[i][1], &max, sizeof(int)); // Write max to pipe
                    }

                    close(pipes[i][1]); // Close writing end in child
                }
                
                return 0; // Exit child process
            } else { 
                if (!with_files) {
                    close(pipes[i][1]); // Close writing end in parent
                }
            }
        } else {
            printf("Fork failed!\n");
            return 1;
        }
    }

    while (active_child_processes > 0) {
        wait(NULL); // Wait for any child process to finish
        active_child_processes -= 1;
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        if (with_files) {
            char filename[BUFFER_SIZE];
            snprintf(filename, sizeof(filename), "result_%d.txt", i / 2); // Use the same file for min/max

            FILE *file = fopen(filename, "r");
            if (!file) {
                perror("Failed to open file");
                return 1;
            }

            int value;
            fscanf(file, "%d", &value); // Read value from file

            if (i % 2 == 0) { // Even indexed processes are min finders
                printf("Child %d found Min: %d\n", i + 1, value);
                if (value < min_max.min) min_max.min = value; 
            } else { // Odd indexed processes are max finders
                printf("Child %d found Max: %d\n", i + 1, value);
                if (value > min_max.max) min_max.max = value; 
            }

            fclose(file); // Close the file
        } else { 
            int value;
            read(pipes[i][0], &value, sizeof(int)); // Read from pipe

            if (i % 2 == 0) { // Even indexed processes are min finders
                printf("Child %d found Min: %d\n", i + 1, value);
                if (value < min_max.min) min_max.min = value; 
            } else { // Odd indexed processes are max finders
                printf("Child %d found Max: %d\n", i + 1, value);
                if (value > min_max.max) min_max.max = value; 
            }
        }
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    
    fflush(NULL);
    
    return 0;
}