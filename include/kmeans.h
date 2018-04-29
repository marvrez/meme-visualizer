#ifndef KMEANS_H
#define KMEANS_H

#include <vector>

typedef struct {
    std::vector<int> assignments;
    matrix centers;
} model_t;

typedef enum {
    L1,
    L2,
    IOU
} kmeans_metric_t;

int* sample(int n);
void random_centers(const matrix_t& data, const matrix_t& centers);


#endif
