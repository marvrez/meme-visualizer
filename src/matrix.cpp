#include "matrix.h"

matrix_t make_matrix(int rows, int cols)
{
    matrix_t m;
    m.rows = rows, m.cols = cols;
    m.vals = std::vector<std::vector<float> >(m.rows, std::vector<float>(m.cols));
    return m;
}

void zero_matrix(matrix_t& m)
{
    for(int i = 0; i < m.rows; ++i) 
    {
        for(int j = 0; j < m.cols; ++j) m.vals[i][j] = 0.f;
    }
}

float mean_matrix(const matrix_t& m) 
{

}

float variance_matrix(const matrix_t& m) 
{

}

matrix_t create_random_matrix(int rows, int cols)
{
    matrix m = make_matrix(rows, cols);
    for (int i = 0; i < rows; ++i) {
        for(int j = 0; j < cols; ++j) {
            m.vals[i][j] = rng1.getFloat();
        }
    }
    return m;
}

int count_fields(std::string line)
{
    int count = 0;
    for(char c : line){
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
    if(!file.good()) {
        fprintf(stderr, "Error: %s\n", filename.c_str());
        exit(0);
    }

    matrix_t m;
    for(std::string line; std::getline(file, line); ) m.vals.push_back(parse_row(line)); 
    m.rows = m.vals.size(), m.cols = m.vals[0].size();
    return m;
}

std::ostream& operator<<(std::ostream& os, const matrix_t& mat) 
{
    for(int i = 0; i < mat.rows; ++i)
    {
        for(int j = 0; j < mat.cols; ++j)
        {
            os << mat.vals[i][j] << " ";
        }
        os << std::endl;
    }
    return os;
}
