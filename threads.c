#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define M 3
#define K 3
#define N 3

int matrix1[M][K] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};

int matrix2[K][N] = {
    {9, 8, 7},
    {6, 5, 4},
    {3, 2, 1}
};

int result[M][N];
pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t barrier_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t barrier_cond = PTHREAD_COND_INITIALIZER;
int threads_in_barrier = 0;

int num_threads;

// Dynamic allocation for local result matrix
int*** local_result;

void barrier() {
    pthread_mutex_lock(&barrier_mutex);
    threads_in_barrier++;
    if (threads_in_barrier == num_threads) {
        threads_in_barrier = 0;
        pthread_cond_broadcast(&barrier_cond);
    } else {
        pthread_cond_wait(&barrier_cond, &barrier_mutex);
    }
    pthread_mutex_unlock(&barrier_mutex);
}

void *multiply(void *arg) {
    int thread_id = *(int *)arg;
    int start_row = thread_id * (M / num_threads);
    int end_row = (thread_id + 1) * (M / num_threads);

    // Dynamic allocation for local result matrix
    local_result[thread_id] = (int**)malloc(M * sizeof(int*));

    for (int i = 0; i < M; i++) {
        local_result[thread_id][i] = (int*)malloc(N * sizeof(int));
        for (int j = 0; j < N; j++) {
            local_result[thread_id][i][j] = 0;
        }
    }

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < K; k++) {
                local_result[thread_id][i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }

    // Wait for all threads to complete their local computations before combining results
    barrier();

    // Combine local results into the global result matrix with proper synchronization
    for (int i = 0; i < M; i++) {
        pthread_mutex_lock(&result_mutex);
        for (int j = 0; j < N; j++) {
            result[i][j] += local_result[thread_id][i][j];
        }
        pthread_mutex_unlock(&result_mutex);
    }

    // Free memory for local result matrix
    for (int i = 0; i < M; i++) {
        free(local_result[thread_id][i]);
    }
    free(local_result[thread_id]);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    num_threads = atoi(argv[1]);

    // Allocate memory for local result matrix array
    local_result = (int***)malloc(num_threads * sizeof(int**));

    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    // Record start time for the entire program
    clock_t program_start_time = clock();

    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, multiply, &thread_ids[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Record end time for the entire program
    clock_t program_end_time = clock();

    // Calculate total execution time for the program
    double program_elapsed_time = ((double)(program_end_time - program_start_time)) / CLOCKS_PER_SEC;

    // Free memory for local result matrix array
    free(local_result);

    // Print the result matrix
    printf("Result Matrix:\n");
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", result[i][j]);
        }
        printf("\n");
    }

    // Print the total program execution time
    printf("\nTotal Program Execution Time: %f seconds\n", program_elapsed_time);

    return EXIT_SUCCESS;
}

