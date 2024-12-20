#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct {
    int start;
    int end;
    long long result;
    pthread_mutex_t *mutex;
} ThreadData;

int k; // число, факториал которого необходимо вычислить
int mod; // модуль
int pnum; // количество потоков

void *calculate_factorial(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    long long local_result = 1;

    for (int i = data->start; i <= data->end; i++) {
        local_result = (local_result * i) % mod;
    }

    pthread_mutex_lock(data->mutex);
    data->result = (data->result * local_result) % mod;
    pthread_mutex_unlock(data->mutex);

    return NULL;
}

int main(int argc, char *argv[]) {
    
    if (argc != 5) {
        fprintf(stderr, "Usage: %s -k <number> --pnum=<threads> --mod=<modulus>\n", argv[0]);
        return 1;
    }

    // Парсинг аргументов командной строки
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0) {
            if (i + 1 < argc) {
                k = atoi(argv[++i]);
            } else {
                fprintf(stderr, "Error: Missing value for -k\n");
                return 1;
            }
        } else if (strncmp(argv[i], "--pnum=", 7) == 0) {
            pnum = atoi(argv[i] + 7);
        } else if (strncmp(argv[i], "--mod=", 6) == 0) {
            mod = atoi(argv[i] + 6);
        } else {
            fprintf(stderr, "Error: Unknown argument %s\n", argv[i]);
            return 1;
        }
    }

    pthread_t threads[pnum];
    ThreadData thread_data[pnum];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    long long result = 1; // общий результат
    int range = k / pnum;

    // Создание потоков
    for (int i = 0; i < pnum; i++) {
        thread_data[i].start = i * range + 1;
        thread_data[i].end = (i == pnum - 1) ? k : (i + 1) * range;
        thread_data[i].result = 1; // начальное значение локального результата
        thread_data[i].mutex = &mutex;

        pthread_create(&threads[i], NULL, calculate_factorial, &thread_data[i]);
    }

    // Ожидание завершения потоков
    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
        result = (result * thread_data[i].result) % mod;
    }

    printf("Factorial of %d modulo %d is: %lld\n", k, mod, result);

    pthread_mutex_destroy(&mutex);
    
    return 0;
}
