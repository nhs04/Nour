#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MIN_VALUE 0
#define MAX_VALUE 5

// Struct to hold thread arguments
typedef struct {
    int* array;
    long long start;
    long long end;
} ThreadArg;

// Thread function to count the number of ones
void* count_ones(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    long long count = 0;
    // Each thread calculates its own local count
    for (long long i = thread_arg->start; i < thread_arg->end; ++i) {
        if (thread_arg->array[i] == 1) {
            ++count;
        }
    }
    
    // Return the count as the exit status
    pthread_exit((void*)count);
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <array_size> <num_threads>\n", argv[0]);
        return 1;
    }

    long long array_size = atoll(argv[1]);
    int num_threads = atoi(argv[2]);

    // Allocate memory for the array
    int* array = malloc(array_size * sizeof(int));
    if (array == NULL) {
        perror("Failed to allocate memory for the array");
        return 1;
    }

    // Generate random integers between 0 and 5
    srand(time(NULL));
    for (long long i = 0; i < array_size; ++i) {
        array[i] = rand() % (MAX_VALUE - MIN_VALUE + 1) + MIN_VALUE;
    }

    // Allocate memory for thread arguments
    ThreadArg* thread_args = malloc(num_threads * sizeof(ThreadArg));
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    if (thread_args == NULL || threads == NULL) {
        perror("Failed to allocate memory for threads or thread arguments");
        free(array);
        return 1;
    }

    // Create threads
    struct timespec start, finish;
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; ++i) {
        thread_args[i].array = array;
        thread_args[i].start = i * (array_size / num_threads);
        thread_args[i].end = (i == num_threads - 1) ? array_size : (i + 1) * (array_size / num_threads);
        if (pthread_create(&threads[i], NULL, count_ones, &thread_args[i])) {
            perror("Failed to create a thread");
            free(array);
            free(thread_args);
            free(threads);
            return 1;
        }
    }

    // Join threads and sum their counts
    long long total_count = 0;
    for (int i = 0; i < num_threads; ++i) {
        void* status;
        pthread_join(threads[i], &status);
        total_count += (long long)status;
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("Number of ones: %lld\n", total_count);
    printf("Time taken: %.3f seconds\n", elapsed);

    // Clean up
    free(array);
    free(thread_args);
    free(threads);

    return 0;
}
