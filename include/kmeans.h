#ifndef KMEANS_H
#define KMEANS_H

#include "matrix.h"

#include <vector>

typedef struct {
    std::vector<int> assignments;
    matrix_t centers;
} model_t;

typedef enum {
    L1,
    L2,
    IOU
} kmeans_metric_t;

int* sample(int n);

// initialization of centroids
void random_centers(const matrix_t& data, const matrix_t& centers);
void smart_centers(const matrix_t& data, const matrix_t& centers);

// return distance to closest centroid measured in given metric
float dist(std::vector<int> x, std::vector<int> y, kmeans_metric_t metric = L2);

// performs the "assignment" steps and assigns each cluster with its nearest centroid 
// returns true if convergence has occurred
bool kmeans_expectation(matrix_t data, model_t* model);

// performs the "update" step of kmeans and assigns new centroids to each cluster
void kmeans_maximization(matrix_t data, model_t* model)

// actual kmeans
model kmeans(matrix_t data, int k, kmeans_metric_t metric);
model soft_kmeans(matrix_t data, int k, kmeans_metric_t metric);

#endif
