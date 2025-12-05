#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"
#include <omp.h>

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <matrix_a> <matrix_b>\n", argv[0]);
        printf("Set OMP_NUM_THREADS environment variable to control threads\n");
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

    // Get thread count for info
    int num_threads;
    #pragma omp parallel
    {
        #pragma omp single
        num_threads = omp_get_num_threads();
    }

    printf("OpenMP Matrix Multiplication: %dx%d * %dx%d = %dx%d\n", 
           matrix_a->rows, matrix_a->cols, matrix_b->rows, matrix_b->cols, 
           result->rows, result->cols);
    printf("Using %d threads\n", num_threads);

    // Time the multiplication
    double start_time = omp_get_wtime();

    // Matrix multiplication with OpenMP
    // Using collapse(2) for better parallelism on larger matrices
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < matrix_a->cols; k++) {
                sum += matrix_a->mat_data[i][k] * matrix_b->mat_data[k][j];
            }
            result->mat_data[i][j] = sum;
        }
    }

    double end_time = omp_get_wtime();

    printf("Time: %.6f seconds\n", end_time - start_time);

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
    free_matrix(matrix_a);
    free_matrix(matrix_b);
    free_matrix(result);

    return 0;
}
