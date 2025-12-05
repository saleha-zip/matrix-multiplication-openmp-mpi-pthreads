#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "matrix.h"

#define DEFAULT_NUM_THREADS 4

// Structure to pass data to threads
typedef struct {
    int thread_id;
    int num_threads;
    matrix_struct *matrix_a;
    matrix_struct *matrix_b;
    matrix_struct *result;
} thread_data_t;

// Thread function
void *matrix_multiply_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    
    int rows = data->result->rows;
    int cols = data->result->cols;
    int inner_dim = data->matrix_a->cols;
    
    // Calculate the portion of rows this thread should process
    int rows_per_thread = rows / data->num_threads;
    int remainder = rows % data->num_threads;
    
    int start_row = data->thread_id * rows_per_thread + 
                   (data->thread_id < remainder ? data->thread_id : remainder);
    int end_row = start_row + rows_per_thread + 
                 (data->thread_id < remainder ? 1 : 0);
    
    // Perform matrix multiplication for assigned rows
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < inner_dim; k++) {
                sum += data->matrix_a->mat_data[i][k] * data->matrix_b->mat_data[k][j];
            }
            data->result->mat_data[i][j] = sum;
        }
    }
    
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <matrix_a> <matrix_b>\n", argv[0]);
        printf("Uses %d threads by default\n", DEFAULT_NUM_THREADS);
        exit(EXIT_FAILURE);
    }

    // Read matrices
    matrix_struct *matrix_a = get_matrix_struct(argv[1]);
    matrix_struct *matrix_b = get_matrix_struct(argv[2]);

    // Validate dimensions
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

    int num_threads = DEFAULT_NUM_THREADS;
    printf("Pthreads Matrix Multiplication: %dx%d * %dx%d = %dx%d\n", 
           matrix_a->rows, matrix_a->cols, matrix_b->rows, matrix_b->cols, 
           result->rows, result->cols);
    printf("Using %d threads\n", num_threads);

    // Allocate thread data and thread IDs
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_data_t *thread_data = malloc(num_threads * sizeof(thread_data_t));

    // Time the multiplication
    clock_t start_time = clock();

    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        thread_data[i].matrix_a = matrix_a;
        thread_data[i].matrix_b = matrix_b;
        thread_data[i].result = result;
        
        pthread_create(&threads[i], NULL, matrix_multiply_thread, &thread_data[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
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
