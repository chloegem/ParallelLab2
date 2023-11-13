#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

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

int main() {
    double start_time, end_time;

    // Set the number of threads
    omp_set_num_threads(4); // Set the desired number of threads

    // Record the start time
    start_time = omp_get_wtime();

    // Parallelized matrix multiplication using OpenMP
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < K; k++) {
                #pragma omp atomic
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }

    // Record the end time
    end_time = omp_get_wtime();

    // Print the result matrix
    printf("Result Matrix:\n");
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", result[i][j]);
        }
        printf("\n");
    }

    // Print the execution time
    printf("Execution Time: %f seconds\n", end_time - start_time);

    return 0;
}
