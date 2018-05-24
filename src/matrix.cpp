#include "matrix.h"

#include <cstdio>

matrix_t make_matrix(int rows, int cols)
{
    matrix_t m;
    m.rows = rows;
    m.cols = cols;
    m.vals = std::vector<std::vector<float> >(m.rows, std::vector<float>(m.cols));
    return m;
}

void zero_matrix(matrix_t* m)
{
    for(int i = 0; i < m->rows; ++i) {
        for(int j = 0; j < m->cols; ++j) m->vals[i][j] = 0.f;
    }
}

float mean_matrix(const matrix_t& m) 
{

}

float variance_matrix(const matrix_t& m) 
{

}

// creates random matrix with uniformly distributed numbers
matrix_t create_random_uniform_matrix(int rows, int cols)
{
    matrix_t m = make_matrix(rows, cols);
    for (int i = 0; i < rows; ++i) 
    {
        for(int j = 0; j < cols; ++j) {
            m.vals[i][j] = rng1.getFloat();
        }
    }
    return m;
}

// create random matrix with gaussian distributed numbers
matrix_t create_random_normal_matrix(int rows, int cols, float mu, float sigma)
{
    matrix_t m = make_matrix(rows, cols);
    static std::mt19937 gen(std::random_device{}());
    std::normal_distribution<float> normal_dist(mu, sigma);
    for(int i = 0; i < m.rows; ++i)
    {
        for(int j = 0; j < m.cols; ++j) {
            m.vals[i][j] = normal_dist(gen);
        }
    }
    return m;
}

void clear_matrix(matrix_t* m)
{
    m->vals.clear();
    m->rows = 0;
    m->cols = 0;
}

matrix_t concat_matrix(matrix_t a, matrix_t b)
{
    int count = 0;
    matrix_t m = make_matrix(a.rows + b.rows, a.cols);
    for(auto val : a.vals) {
        m.vals[count++] = val;
    }
    for(auto val : b.vals) {
        m.vals[count++] = val;
    }
    return m;
}

int count_fields(std::string line)
{
    int count = 0;
    for(char c : line) {
        if(c == ',') ++count;
    }
    return count+1;
}

std::vector<float> parse_row(std::string line)
{
    std::vector<float> values;
    std::stringstream ss(line);

    for(float val; ss >> val; )
    {
        values.push_back(val);

        if (ss.peek() == ',')
        ss.ignore();
    }

    return values;
}

matrix_t csv_to_matrix(std::string filename)
{
    std::ifstream file(filename);
    if(!file.good()) 
    {
        fprintf(stderr, "Error: %s\n", filename.c_str());
        exit(0);
    }

    matrix_t m;
    for(std::string line; std::getline(file, line); ) m.vals.push_back(parse_row(line)); 
    m.rows = m.vals.size(), m.cols = m.vals[0].size();
    return m;
}

void print_matrix(const matrix_t& m) 
{
    printf("%d X %d Matrix:\n",m.rows, m.cols);
    printf(" __");
    for(int j = 0; j < 16*m.cols-1; ++j) printf(" ");
    printf("__ \n");

    printf("|  ");
    for(int j = 0; j < 16*m.cols-1; ++j) printf(" ");
    printf("  |\n");

    for(int i = 0; i < m.rows; ++i)
    {
        printf("|  ");
        for(int j = 0; j < m.cols; ++j) {
            printf("%15.7f ", m.vals[i][j]);
        }
        printf(" |\n");
    }
    printf("|__");
    for(int j = 0; j < 16*m.cols-1; ++j) printf(" ");
    printf("__|\n");
}
