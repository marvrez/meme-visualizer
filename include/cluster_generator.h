#ifndef CLUSTER_GENERATOR_H
#define CLUSTER_GENERATOR_H

#include <vector>
#include <random>
#include <iostream>

float normal_dist(float mu, float sigma) 
{
    static std::mt19937 gen(std::random_device{}());
    std::normal_distribution<float> distr(mu, sigma);
    return distr(gen);
}

// Primary cluster generator - aborts if no centroids have been imported.
std::vector<matrix_t> generate_clusters(matrix_t centroids, const int max_num_points, const float sigma) 
{
    if (centroids.vals.empty()) 
    {
        std::cout << "[ERROR] No centroids have been imported. Aborting operation.";
        return {};
    }

    std::vector<matrix_t> clusters;
    for (auto centroid_iter = centroids.vals.begin(); centroid_iter != centroids.vals.end(); ++centroid_iter) 
    {
        matrix_t cluster;
        for(int i = 0; i < rng0.Rand(max_num_points) + 50; ++i) 
        {
            std::vector<float> temp_point;
            for (auto dimension_iter = centroid_iter->begin(); dimension_iter != centroid_iter->end(); ++dimension_iter) {
                temp_point.push_back(normal_dist(*dimension_iter, sigma));
            }
            cluster.vals.push_back(temp_point);
        }
        cluster.rows = cluster.vals.size(), cluster.cols = cluster.vals[0].size();
        clusters.push_back(cluster);
    }
    return clusters;
}

#endif
