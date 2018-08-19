#ifndef MATRIX_H
#define MATRIX_H

#include <string>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <ostream>
#include <vector>

#include "rng.h"

typedef struct {
    int rows, cols;
    std::vector<std::vector<float> > vals;
} matrix_t;

// create matrix
matrix_t make_matrix(int rows, int cols);
matrix_t make_identity(int n);
void zero_matrix(matrix_t* m);
matrix_t create_random_uniform_matrix(int rows, int cols);
matrix_t create_random_normal_matrix(int rows, int cols, float mu = 0, float sigma = 1);

void clear_matrix(matrix_t* m);
matrix_t concat_matrix(matrix_t a, matrix_t b);

// statistics stuff
float mean_matrix(const matrix_t& m);
float variance_matrix(const matrix_t& m);
matrix_t covariance_matrix(const matrix_t& m);

std::vector<float> get_diagonal(const matrix_t& m);

// utils for parsing from file
int count_fields(std::string line);
std::vector<float> parse_row(std::string line);
matrix_t csv_to_matrix(std::string filename);
void print_matrix(const matrix_t& m);

// matrix operators
matrix_t operator+(const matrix_t& a, const matrix_t& b);
matrix_t operator-(const matrix_t& a, const matrix_t& b);
matrix_t operator*(const matrix_t& a, const matrix_t& b);
matrix_t transpose_matrix(const matrix_t& m);
matrix_t elmult_matrix(const matrix_t& a, const matrix_t& b);
void scale_matrix(matrix_t* m, float scale_val);

// more specialized linalg stuff
void jacobi_eigenvalue(matrix_t* m, std::vector<float>& eigen_vals, matrix_t* eigen_vecs, int max_iter = 100);

#endif
