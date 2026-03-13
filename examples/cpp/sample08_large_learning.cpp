//
// Created by mohammad on 3/13/26.
//
#include <iostream>

#include "common.h"
#include "libgsp/utils/Logging.h"
#include "libgsp/Graph.h"
#include "libgsp/learning/LearningCore.h"


int main(int argc, char** argv) {
    gsp::logging::basicConfig(argc, argv, gsp::logging::level::trace);
    auto logger = gsp::logging::getLogger();

    logger->info("Sample 08 - large graph learning examples");

    // Generate a large-scale synthetic signal: 100k samples, 3 features
    int n_samples = 10'000'000;
    int n_features = 3;
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> X(n_samples, n_features);
    X.setRandom(); // Fill with random values

    // Compute pairwise distances using ANN (Approximate Nearest Neighbors)
    // This is efficient for large-scale data compared to full pairwise computation
    gsp::NanoflannAnnDistance<double> ann;

    int k = 5;
    int r = 2;
    // Calculate sparse distance matrix (L2 distance, k=10 neighbors)
    auto sparse_distance = ann.setMetric(gsp::DistanceMetric::L2Distance)
            .setKFixed(k*r)
            .setExcludeSelf(false)
            .setTriangularOnly(false)
            .compute(X);

    // Convert distances to squared distances (Z) as required by the algorithm
    gsp::sparsematrix Z = sparse_distance.cwisePow(2);

    // Initialize the graph learner
    gsp::GraphLearningLogDegrees<decltype(Z)> learner;

    // Log the start of the large-scale test
    logger->info("Starting Large-Scale Graph Learning Test on 100k nodes...");


    // Run the graph learning optimization
    // autoCompute handles the setup of alpha/beta based on theta if implemented
    auto W = learner.setMaxIterations(30)
            .setVerbosity(2)
            .autoCompute(Z, k);

    // Log the result (showing a subset or stats to avoid console flood)
    logger->info("Graph Learning completed. Result W shape: {}x{}", W.rows(), W.cols());
    return 0;
}