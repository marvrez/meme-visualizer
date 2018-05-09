#include "kmeans.h"

#include <cassert>
#include <array>
#include <algorithm>

void random_centers(const matrix_t& data, matrix_t* centers)
{
    printf("data: %d, centers: %d\n", data.cols, centers->cols);
    assert(centers->cols == data.cols);

    // create index array and shuffle it
    std::vector<int> index_array(data.rows);
    for(int i = 0; i < index_array.size(); ++i) {
        index_array[i] = i;
    }
    std::random_shuffle(index_array.begin(), index_array.end());

    for(int i = 0; i < centers->rows; ++i) {
        centers->vals[i] = data.vals[index_array[i]];
    }
}

void smart_centers(const matrix_t& data, matrix_t* centers)
{
    //TODO
}
