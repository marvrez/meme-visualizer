#include "kmeans.h"
#include "rng.h"

#include <cassert>
#include <array>
#include <cstdio>
#include <algorithm>

void random_centers(const matrix_t& data, matrix_t* centers)
{
    assert(centers->cols == data.cols);

    // get index of arrays and shuffle them
    std::vector<int> index_array(data.rows);
    for(int i = 0; i < index_array.size(); ++i) {
        index_array[i] = i;
    }
    std::random_shuffle(index_array.begin(), index_array.end());

    for(int i = 0; i < centers->rows; ++i) {
        centers->vals[i] = data.vals[index_array[i]];
    }
}

void smart_centers(const matrix_t& data, matrix_t* centers, kmeans_metric_t metric)
{
    assert(data.cols == centers->cols);

    RNG rng(0, data.rows);
    centers->vals[0] = data.vals[rng.getInt()];

    std::vector<float> closest_dist_to_centroid(data.rows, 0.f);
    int num_centers = centers->rows;
    for (int i = 1; i < num_centers; ++i) {
        float sum = 0;
        centers->rows = i;
        for (int j = 0; j < data.rows; ++j) {
            closest_dist_to_centroid[j] = get_closest_center(data.vals[j], *centers, metric).second;
            sum += closest_dist_to_centroid[j];
        }
        float r = sum * rng0.getFloat();
        for (int j = 0; j < data.rows; ++j) {
            r -= closest_dist_to_centroid[j];
            if(r <= 0) {
                centers->vals[i] = data.vals[j];
                break;
            }
        }
    }
    centers->rows = num_centers;
}

float dist(std::vector<float> x, std::vector<float> y, kmeans_metric_t metric)
{
    assert(x.size() == y.size());
    const int n = x.size();

    float dist = 0;
    switch (metric)
    {
        case IOU: {
            // assumes that first index is box width and second index is box height
            float min_w = std::min(x[0], y[0]), min_h = std::min(x[1], y[1]);
            float box_intersect = min_w * min_h;
            float box_union = (x[0] * x[1] + y[0] * y[1]) - box_intersect;
            float iou = box_intersect / box_union;
            dist = 1 - iou;
            break;
        }
        case L1: {
            for(int i = 0; i < n; ++i) dist += fabs(x[i]-y[i]);
            break;
        }
        case L2: {
            for(int i = 0; i < n; ++i) dist += (x[i]-y[i])*(x[i]-y[i]);
            dist = sqrt(dist);
            break;
        }
    }

    return dist;
}

std::pair<int, float> get_closest_center(const std::vector<float>& data, const matrix_t& centers, kmeans_metric_t metric)
{
    int closest_center = 0;
    float closest_dist = dist(data, centers.vals[closest_center], metric);
    for(int i = 0; i < centers.rows; ++i) {
        float cur_dist = dist(data, centers.vals[i], metric);
        if(cur_dist < closest_dist) {
            closest_dist = cur_dist;
            closest_center = i;
        }
    }
    return std::make_pair(closest_center, closest_dist);
}

bool kmeans_expectation(matrix_t data, model_t* model, kmeans_metric_t metric)
{
    bool converged = true;
    for(int i = 0; i < data.rows; ++i) {
        auto closest = get_closest_center(data.vals[i], model->centers, metric);
        float closest_center_idx = closest.first;
        if(closest_center_idx != model->assignments[i]) converged = false;
        model->assignments[i] = closest_center_idx;
    }
    return converged;
}

void kmeans_maximization(matrix_t data, model_t* model)
{
    std::vector<int> counts(model->centers.rows, 0);
    zero_matrix(&model->centers);
    for(int i = 0; i < data.rows; ++i) {
        counts[model->assignments[i]]++;
        for(int j = 0; j < data.cols; ++j){
            model->centers.vals[model->assignments[i]][j] += data.vals[i][j];
        }
    }
    for(int i = 0; i < model->centers.rows; ++i) { 
        if(counts[i]) {
            for(int j = 0; j < model->centers.cols; ++j) {
                model->centers.vals[i][j] /= counts[i];
            }
        }
    }
}

model_t kmeans(matrix_t data, int k, kmeans_metric_t metric, bool use_smart_centers)
{
    if(metric == IOU) {
        for(int i = 0; i < data.rows; ++i) {
            for(int j = 0 ; j < data.cols; ++j) 
                assert(data.vals[i][j] > 0);
        }
    }

    matrix_t centers = make_matrix(k, data.cols);
    std::vector<int> assignments(data.rows, 0);
    model_t model = {assignments, centers};

    if(use_smart_centers) smart_centers(data, &model.centers, metric);
    else random_centers(data, &model.centers);

    if(k == 1) kmeans_maximization(data, &model);
    while(!kmeans_expectation(data, &model, metric)) {
        kmeans_maximization(data, &model);
    }

    return model;
}
