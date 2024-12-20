#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

void *thread1_func(void *arg) {
    // Захватываем первый мьютекс
    pthread_mutex_lock(&mutex1);
    printf("Thread 1: locked mutex 1\n");

    // Ждем немного, чтобы гарантировать, что второй поток успеет захватить второй мьютекс
    sleep(1);

    // Пытаемся захватить второй мьютекс
    printf("Thread 1: trying to lock mutex 2\n");
    pthread_mutex_lock(&mutex2);
    printf("Thread 1: locked mutex 2\n");

    // Освобождаем мьютексы
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

void *thread2_func(void *arg) {
    // Захватываем второй мьютекс
    pthread_mutex_lock(&mutex2);
    printf("Thread 2: locked mutex 2\n");

    // Ждем немного, чтобы гарантировать, что первый поток успеет захватить первый мьютекс
    sleep(1);

    // Пытаемся захватить первый мьютекс
    printf("Thread 2: trying to lock mutex 1\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 2: locked mutex 1\n");

    // Освобождаем мьютексы
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);

    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Инициализация мьютексов
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);

    // Создание потоков
    pthread_create(&thread1, NULL, thread1_func, NULL);
    pthread_create(&thread2, NULL, thread2_func, NULL);

    // Ожидание завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Уничтожение мьютексов
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);

    return 0;
}
