#ifndef KMEANS_H
#define KMEANS_H

#include <vector>

typedef struct {
    std::vector<int> assignments;
    matrix centers;
} model_t;

int* sample(int n)
{

}

void random_centers(const matrix_t& data, const matrix_t& centers)
{
    int* s = sample(data.rows);
    for(int i = 0; i < centers.rows; ++i){
        copy(data.vals[s[i]], centers.vals[i], data.cols);
    }
}


#endif
