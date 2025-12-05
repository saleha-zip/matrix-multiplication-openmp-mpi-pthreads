#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "matrix.h"

// Structure to pass data to threads
typedef struct {
    int thread_id;
    int num_threads;
    int start_row;
    int end_row;
    matrix_struct *matrix_a;
    matrix_struct *matrix_b;
    matrix_struct *result;
} thread_data_t;

void *worker(void *param); /* the thread */

int main(int argc, char **argv)
{
    int num_procs = sysconf(_SC_NPROCESSORS_ONLN);

    if (argc != 3) {
        printf("Usage: %s <matrix_a> <matrix_b>\n", argv[0]);
        printf("Automatically using %d threads (number of CPU cores)\n", num_procs);
        exit(EXIT_FAILURE);
    }

    // Read matrices
    matrix_struct *matrix_a = get_matrix_struct(argv[1]);
    matrix_struct *matrix_b = get_matrix_struct(argv[2]);

    if (matrix_a->cols != matrix_b->rows) {
        printf("Error: Matrix dimensions incompatible for multiplication\n");
        printf("A: %dx%d, B: %dx%d\n", matrix_a->rows, matrix_a->cols, matrix_b->rows, matrix_b->cols);
        free_matrix(matrix_a);
        free_matrix(matrix_b);
        exit(EXIT_FAILURE);
    }

    // Allocate result matrix
    matrix_struct *result = malloc(sizeof(matrix_struct));
    result->rows = matrix_a->rows;
    result->cols = matrix_b->cols;
    result->mat_data = malloc(result->rows * sizeof(double *));
    for (int i = 0; i < result->rows; i++) {
        result->mat_data[i] = calloc(result->cols, sizeof(double));
    }

    printf("Pthreads Matrix Multiplication: %dx%d * %dx%d = %dx%d\n", 
           matrix_a->rows, matrix_a->cols, matrix_b->rows, matrix_b->cols, 
           result->rows, result->cols);
    printf("Using %d threads (CPU cores)\n", num_procs);

    // Allocate thread handles and data
    pthread_t *threads = malloc(num_procs * sizeof(pthread_t));
    thread_data_t *thread_data = malloc(num_procs * sizeof(thread_data_t));

    // Calculate rows per thread
    int rows_per_thread = result->rows / num_procs;
    int remainder = result->rows % num_procs;

    // Time the multiplication
    clock_t start_time = clock();

    // Create threads
    for (int i = 0; i < num_procs; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_procs;
        thread_data[i].matrix_a = matrix_a;
        thread_data[i].matrix_b = matrix_b;
        thread_data[i].result = result;
        
        // Calculate start and end rows for this thread
        thread_data[i].start_row = i * rows_per_thread + (i < remainder ? i : remainder);
        thread_data[i].end_row = thread_data[i].start_row + rows_per_thread + 
                                (i < remainder ? 1 : 0);
        
        pthread_create(&threads[i], NULL, worker, &thread_data[i]);
    }

    // Join all threads
    for (int i = 0; i < num_procs; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t end_time = clock();
    double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    printf("Time: %.6f seconds\n", cpu_time_used);

    // Print result only for small matrices
    if (result->rows <= 10 && result->cols <= 10) {
        printf("Result:\n");
        print_matrix(result);
    } else {
        printf("Result matrix too large to display (%dx%d)\n", result->rows, result->cols);
        // Print a sample of the result
        printf("Sample - top-left 3x3:\n");
        for (int i = 0; i < 3 && i < result->rows; i++) {
            for (int j = 0; j < 3 && j < result->cols; j++) {
                printf("%8.2f ", result->mat_data[i][j]);
            }
            printf("\n");
        }
    }

    // Cleanup
    free(threads);
    free(thread_data);
    free_matrix(matrix_a);
    free_matrix(matrix_b);
    free_matrix(result);

    return 0;
}

void *worker(void *param) {
    thread_data_t *data = (thread_data_t *)param;
    
    // Perform matrix multiplication for assigned rows
    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < data->result->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < data->matrix_a->cols; k++) {
                sum += data->matrix_a->mat_data[i][k] * data->matrix_b->mat_data[k][j];
            }
            data->result->mat_data[i][j] = sum;
        }
    }

    pthread_exit(0);
}
