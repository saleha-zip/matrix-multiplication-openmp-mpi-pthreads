#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"


// Allocate and read a matrix from a file
matrix_struct *get_matrix_struct(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // First pass: count rows and columns
    int rows = 0, cols = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), file)) {
        rows++;
        int current_cols = 0;
        char *ptr = line;
        double value;
        
        // Count numbers in this line
        while (sscanf(ptr, "%lf", &value) == 1) {
            current_cols++;
            // Move pointer past the number we just read
            char *end;
            strtod(ptr, &end);
            ptr = end;
        }
        
        if (rows == 1) {
            cols = current_cols; // Set columns based on first line
        } else if (current_cols != cols) {
            fprintf(stderr, "Error: Inconsistent number of columns in row %d\n", rows);
            fclose(file);
            exit(EXIT_FAILURE);
        }
    }

    rewind(file);

    // Allocate matrix
    matrix_struct *m = malloc(sizeof(matrix_struct));
    m->rows = rows;
    m->cols = cols;
    m->mat_data = malloc(rows * sizeof(double *));
    for (int i = 0; i < rows; i++) {
        m->mat_data[i] = malloc(cols * sizeof(double));
    }

    // Second pass: read the data
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (fscanf(file, "%lf", &m->mat_data[i][j]) != 1) {
                fprintf(stderr, "Error reading matrix data at row %d, col %d\n", i, j);
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(file);
    return m;
}


// Print matrix to stdout
void print_matrix(matrix_struct *m) {
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++)
            printf("%lf\t", m->mat_data[i][j]);
        printf("\n");
    }
}

// Free matrix memory
void free_matrix(matrix_struct *m) {
    for (int i = 0; i < m->rows; i++)
        free(m->mat_data[i]);
    free(m->mat_data);
    free(m);
}

