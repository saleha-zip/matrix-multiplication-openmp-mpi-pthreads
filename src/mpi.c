#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "matrix.h"

int main(int argc, char *argv[]) {
    int num_procs, rank;
    matrix_struct *matrix_a = NULL, *matrix_b = NULL, *result = NULL;
    double *flat_a = NULL, *flat_b = NULL, *flat_result = NULL;
    double start_time = 0.0, end_time = 0.0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Master process reads matrices
    if (rank == 0) {
        if (argc != 3) {
            printf("Usage: mpirun -n <processes> ./mpi <matrix_a> <matrix_b>\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        matrix_a = get_matrix_struct(argv[1]);
        matrix_b = get_matrix_struct(argv[2]);

        if (matrix_a->cols != matrix_b->rows) {
            printf("Error: Matrix dimensions incompatible for multiplication\n");
            printf("A: %dx%d, B: %dx%d\n", matrix_a->rows, matrix_a->cols, matrix_b->rows, matrix_b->cols);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Allocate result matrix
        result = malloc(sizeof(matrix_struct));
        result->rows = matrix_a->rows;
        result->cols = matrix_b->cols;
        result->mat_data = malloc(result->rows * sizeof(double *));
        for (int i = 0; i < result->rows; i++) {
            result->mat_data[i] = calloc(result->cols, sizeof(double));
        }

        start_time = MPI_Wtime();
    }

    // Broadcast matrix dimensions
    int dims[4];
    if (rank == 0) {
        dims[0] = matrix_a->rows;
        dims[1] = matrix_a->cols;
        dims[2] = matrix_b->rows;
        dims[3] = matrix_b->cols;
    }
    MPI_Bcast(dims, 4, MPI_INT, 0, MPI_COMM_WORLD);

    int rows_a = dims[0], cols_a = dims[1];
    int rows_b = dims[2], cols_b = dims[3];
    int rows_result = rows_a, cols_result = cols_b;

    // Flatten matrices for distribution
    int size_a = rows_a * cols_a;
    int size_b = rows_b * cols_b;
    int size_result = rows_result * cols_result;

    if (rank == 0) {
        flat_a = malloc(size_a * sizeof(double));
        flat_b = malloc(size_b * sizeof(double));
        flat_result = malloc(size_result * sizeof(double));

        // Flatten matrix A
        for (int i = 0; i < rows_a; i++) {
            for (int j = 0; j < cols_a; j++) {
                flat_a[i * cols_a + j] = matrix_a->mat_data[i][j];
            }
        }

        // Flatten matrix B
        for (int i = 0; i < rows_b; i++) {
            for (int j = 0; j < cols_b; j++) {
                flat_b[i * cols_b + j] = matrix_b->mat_data[i][j];
            }
        }
    } else {
        flat_a = malloc(size_a * sizeof(double));
        flat_b = malloc(size_b * sizeof(double));
        flat_result = malloc(size_result * sizeof(double));
    }

    // Broadcast matrices to all processes
    MPI_Bcast(flat_a, size_a, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(flat_b, size_b, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Calculate local portion
    int rows_per_proc = rows_result / num_procs;
    int remainder = rows_result % num_procs;
    
    int start_row = rank * rows_per_proc + (rank < remainder ? rank : remainder);
    int end_row = start_row + rows_per_proc + (rank < remainder ? 1 : 0);
    
    int local_rows = end_row - start_row;

    // Local computation
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < cols_result; j++) {
            double sum = 0.0;
            for (int k = 0; k < cols_a; k++) {
                sum += flat_a[i * cols_a + k] * flat_b[k * cols_b + j];
            }
            flat_result[i * cols_result + j] = sum;
        }
    }

    // Gather results
    int *recv_counts = NULL;
    int *displs = NULL;

    if (rank == 0) {
        recv_counts = malloc(num_procs * sizeof(int));
        displs = malloc(num_procs * sizeof(int));

        for (int i = 0; i < num_procs; i++) {
            int start = i * rows_per_proc + (i < remainder ? i : remainder);
            int end = start + rows_per_proc + (i < remainder ? 1 : 0);
            recv_counts[i] = (end - start) * cols_result;
            displs[i] = start * cols_result;
        }
    }

    MPI_Gatherv(&flat_result[start_row * cols_result], local_rows * cols_result, MPI_DOUBLE,
                flat_result, recv_counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Master process prints result and timing
    if (rank == 0) {
        end_time = MPI_Wtime();

        // Convert flat result back to 2D
        for (int i = 0; i < rows_result; i++) {
            for (int j = 0; j < cols_result; j++) {
                result->mat_data[i][j] = flat_result[i * cols_result + j];
            }
        }

        printf("MPI Matrix Multiplication: %dx%d * %dx%d = %dx%d\n", 
               rows_a, cols_a, rows_b, cols_b, rows_result, cols_result);
        printf("Time: %.6f seconds\n", end_time - start_time);
        
        // Print result for small matrices
        if (rows_result <= 10 && cols_result <= 10) {
            printf("Result:\n");
            print_matrix(result);
        }

        // Cleanup
        free_matrix(matrix_a);
        free_matrix(matrix_b);
        free_matrix(result);
        free(recv_counts);
        free(displs);
    }

    free(flat_a);
    free(flat_b);
    free(flat_result);

    MPI_Finalize();
    return 0;
}
