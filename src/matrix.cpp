#include "matrix.h"

#include <cstdio>
#include <cassert>

matrix_t make_matrix(int rows, int cols)
{
    matrix_t m;
    m.rows = rows;
    m.cols = cols;
    m.vals = std::vector<std::vector<float> >(m.rows, std::vector<float>(m.cols));
    return m;
}

matrix_t make_identity(int n)
{
    matrix_t m = make_matrix(n, n);
    for(int i = 0; i < n; ++i) {
        m.vals[i][i] = 1;
    }
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

matrix_t covariance_matrix(const matrix_t& m)
{
    const int dim = m.cols, num_data_points = m.rows;
    assert(num_data_points > 0 && dim > 0);

    std::vector<float> mean(dim, 0);
    matrix_t cov_mat = make_matrix(dim, dim);

    for (int i = 0; i < dim; ++i) {
        for (int k = 0; k < num_data_points; ++k) mean[i] += m.vals[k][i];
        mean[i] /= num_data_points;
    }

    for (int k = 0; k < num_data_points; ++k)
        for (int i = 0; i < dim; ++i)
            for (int j = 0; j <= i; ++j)
                cov_mat.vals[i][j] += (m.vals[k][i] - mean[i]) * (m.vals[k][j] - mean[j]);

    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < i; ++j) {
            cov_mat.vals[i][j] /= num_data_points;
            cov_mat.vals[j][i] = cov_mat.vals[i][j];
        }
        cov_mat.vals[i][i] /= num_data_points;
    }

    return cov_mat;
}


std::vector<float> get_diagonal(const matrix_t& m)
{
    assert(m.rows == m.cols);
    std::vector<float> v(m.rows);
    for(int i = 0; i < m.rows; ++i) v[i] = m.vals[i][i];
    return v;
}

// creates random matrix with uniformly distributed numbers
matrix_t create_random_uniform_matrix(int rows, int cols)
{
    matrix_t m = make_matrix(rows, cols);
    for (int i = 0; i < rows; ++i) {
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
    for(int i = 0; i < m.rows; ++i) {
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
    return count + 1;
}

std::vector<float> parse_row(std::string line)
{
    std::vector<float> values;
    std::stringstream ss(line);

    for(float val; ss >> val; ) {
        values.push_back(val);

        if (ss.peek() == ',')
        ss.ignore();
    }

    return values;
}

matrix_t csv_to_matrix(std::string filename)
{
    std::ifstream file(filename);
    if(!file.good()) {
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

    for(int i = 0; i < m.rows; ++i) {
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

matrix_t operator+(const matrix_t& a, const matrix_t& b)
{
    assert(a.cols == b.cols);
    assert(a.rows == b.rows);
    matrix_t p = make_matrix(a.rows, a.cols);
    for(int i = 0; i < p.rows; ++i) {
        for(int j = 0; j < p.cols; ++j) {
            p.vals[i][j] = a.vals[i][j] + b.vals[i][j];
        }
    }
    return p;
}

matrix_t operator-(const matrix_t& a, const matrix_t& b)
{
    assert(a.cols == b.cols);
    assert(a.rows == b.rows);
    matrix_t result = make_matrix(a.rows, a.cols);
    for(int i = 0; i < result.rows; ++i) {
        for(int j = 0; j < result.cols; ++j) {
            result.vals[i][j] = a.vals[i][j] - b.vals[i][j];
        }
    }
    return result;
}

matrix_t operator*(const matrix_t& a, const matrix_t& b)
{
    assert(a.cols == b.rows);
    matrix_t result = make_matrix(a.rows, b.cols);
    for(int i = 0; i < result.rows; ++i) {
        for(int j = 0; j < result.cols; ++j) {
            for(int k = 0; k < a.cols; ++k) {
                result.vals[i][j] += a.vals[i][k] * b.vals[k][j];
            }
        }
    }
    return result;
}

matrix_t transpose_matrix(const matrix_t& m)
{
    matrix_t t = make_matrix(m.rows, m.cols);
    for(int i = 0; i < t.rows; ++i) {
        for(int j = 0; j < t.cols; ++j) {
            t.vals[i][j] = m.vals[j][i];
        }
    }
    return t;
}

matrix_t matrix_elmult_matrix(const matrix_t& a, const matrix_t& b)
{
    assert(a.cols == b.cols);
    assert(a.rows == b.rows);
    matrix_t result = make_matrix(a.rows, a.cols);
    for(int i = 0; i < result.rows; ++i){
        for(int j = 0; j < result.cols; ++j){
            result.vals[i][j] = a.vals[i][j] * b.vals[i][j];
        }
    }
    return result;
}

void scale_matrix(matrix_t* m, float scale_val)
{
    for(int i = 0; i < m->rows; ++i){
        for(int j = 0 ; j < m->cols; ++j){
            m->vals[i][j] *= scale_val;
        }
    }
}

void jacobi_eigenvalue(matrix_t* m, std::vector<float>& eigen_vals, matrix_t* eigen_vecs, int max_iter)
{
    int n = m->cols;
    std::vector<float> b(n, 0), z(n, 0);

    *eigen_vecs = make_identity(n);
    for (int i = 0; i < n; ++i) {
        b[i] = eigen_vals[i] = m->vals[i][i];
        z[i] = 0.0f;
    }

    for (int i = 1; i <= max_iter; ++i) {
        float thresh = 0.0f;
        for (int p = 0; p < n - 1; ++p) {
            for (int q = p + 1; q < n; ++q) {
                thresh += m->vals[p][q] * m->vals[p][q];
            }
        }

        thresh = sqrtf(thresh) / (float) (4*n); 
        if (thresh == 0.0f) break;

        for (int p = 0; p < n; ++p) {
            for (int q = p + 1; q < n; ++q) {
                float g = 10.0f * fabs(m->vals[p][q]);

                //  Annihilate tiny offdiagonal elements.
                if (i > 4 && fabs(eigen_vals[p]) + g == fabs(eigen_vals[p]) && fabs(eigen_vals[q]) + g == fabs(eigen_vals[q]))
                    m->vals[p][q] = 0.0;

                // otherwise, apply a rotation
                else if (thresh <= fabs(m->vals[p][q])) {
                    float tau, t, s, c;
                    float h = eigen_vals[q] - eigen_vals[p];

                    if (fabs(h) + g == fabs(h)) t = (m->vals[p][q]) / h;
                    else {
                        float theta = 0.5f * h / (m->vals[p][q]);
                        t = 1.0f / (fabs(theta) + sqrtf(1.0f + theta * theta));
                        if (theta < 0.0f) t = -t;
                    }
                    c = 1.0f / sqrtf(1.0f + t * t);
                    s = t * c;
                    tau = s / (1.0f + c);
                    h = t * m->vals[p][q];

                    //  Accumulate corrections to diagonal elements.
                    z[p] -= h, z[q] += h;
                    eigen_vals[p] -= h, eigen_vals[q] += h;
                    m->vals[p][q] = 0.0f;

                    #define m_rotate(a,i,j,k,l) \
                        g = a[i][j]; \
                        h = a[k][l]; \
                        a[i][j] = g - s*(h + g*tau); \
                        a[k][l] = h + s*(g - h*tau);

                    //  Rotate, using information from the upper triangle of A only.
                    for (int j = 0; j < p; ++j) {
                        m_rotate(m->vals, j, p, j, q)
                    }
                    for (int j = p + 1; j < q; ++j) {
                        m_rotate(m->vals, p, j, j, q)
                    }
                    for (int j = q + 1; j < n; ++j) {
                        m_rotate(m->vals, p, j, q, j)
                    }

                    //  Accumulate information in the eigenvector matrix.
                    for (int j = 0; j < n; ++j) {
                        m_rotate(eigen_vecs->vals, p, j, q, j)
                    }
                    #undef m_rotate
                }
            }
        }

        for (int j = 0; j < n; ++j) {
            b[j] += z[j];
            eigen_vals[j] = b[j];
            z[j]  = 0.0f;
        }
    }

    //  Restore upper triangle of input matrix.
    for (int p = 0; p < n - 1; ++p) {
        for (int q = p + 1; q < n; ++q) {
            m->vals[p][q] = m->vals[q][p];
        }
    }
}
