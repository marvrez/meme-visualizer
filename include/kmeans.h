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

// initialization of centroids
void random_centers(const matrix_t& data, matrix_t* centers);
void smart_centers(const matrix_t& data, matrix_t* centers);

// return distance to closest centroid measured in given metric
// x is data point(box for IoU), y is the centroid(anchor for IoU)
float dist(std::vector<float> x, std::vector<float> y, kmeans_metric_t metric = L2);

// Returns index of the closest center and the distance from given data to that center
std::pair<int,float> get_closest_center(const std::vector<float>& data, const matrix_t& centers, kmeans_metric_t metric = L2);

// performs the "assignment" steps and assigns each cluster with its nearest centroid 
// returns true if convergence has occurred
bool kmeans_expectation(matrix_t data, model_t* model, kmeans_metric_t metric = L2);

// performs the "update" step of kmeans and assigns new centroids to each cluster
void kmeans_maximization(matrix_t data, model_t* model);

// actual kmeans
model_t kmeans(matrix_t data, int k, kmeans_metric_t metric);
model_t soft_kmeans(matrix_t data, int k, kmeans_metric_t metric);

#endif
