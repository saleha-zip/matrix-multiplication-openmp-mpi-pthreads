#ifndef MATRIX_H
#define MATRIX_H

typedef struct {
    int rows;
    int cols;
    double **mat_data;
} matrix_struct;

matrix_struct *get_matrix_struct(const char *filename);
void print_matrix(matrix_struct *matrix_to_print);
void free_matrix(matrix_struct *matrix_to_free);

#endif

