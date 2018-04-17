#ifndef MATRIX_H
#define MATRIX_H

#include <string>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <ostream>
#include <vector>

#include "rng.h"

typedef struct matrix {
    int rows, cols;
    std::vector<std::vector<float> > vals;
} matrix_t;

// create matrix
matrix_t make_matrix(int rows, int cols);
void zero_matrix(matrix_t& m);
matrix_t create_random_matrix(int rows, int cols);

// statistics stuff
float mean_matrix(const matrix_t& m);
float variance_matrix(const matrix_t& m);

// utils for parsing from file
int count_fields(std::string line);
std::vector<float> parse_row(std::string line);
matrix_t csv_to_matrix(std::string filename);
std::ostream& operator<<(std::ostream& os, const matrix_t& mat);

#endif
