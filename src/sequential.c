#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "matrix.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <matrix_a> <matrix_b>\n", argv[0]);
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

    printf("Sequential Matrix Multiplication: %dx%d * %dx%d = %dx%d\n", 
           matrix_a->rows, matrix_a->cols, matrix_b->rows, matrix_b->cols, 
           result->rows, result->cols);

    // Time the multiplication
    clock_t start_time = clock();

    // Sequential matrix multiplication
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < matrix_a->cols; k++) {
                sum += matrix_a->mat_data[i][k] * matrix_b->mat_data[k][j];
            }
            result->mat_data[i][j] = sum;
        }
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
    free_matrix(matrix_a);
    free_matrix(matrix_b);
    free_matrix(result);

    return 0;
}
