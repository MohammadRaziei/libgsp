//
// Created by mohammad on 1/13/26.
//
#include <iostream>

#include "common.h"
#include "libgsp/utils/Logging.h"
#include "libgsp/Graph.h"
#include "libgsp/learning/Distance.h"
#include "libgsp/learning/Kernels.h"


int main(int argc, char** argv) {
    gsp::logging::basicConfig(argc, argv, gsp::logging::level::trace);
    auto logger = gsp::logging::getLogger();

    logger->info("Sample 07 - graph learning examples");

    std::vector<gsp::Edge>  edges      = {{0, 0},{0,1, 3},{0,2},{1,2},{2,3}};
    std::vector<gsp::Coord> coords_vec = {{0,0}, {2,0}, {1,-1}, {3,-1}};

    gsp::DenseGraph graph(4);
    graph.setEdges(edges, true);
    graph.setCoords(coords_vec);

//    logger->info("graph W: \n{}", fmt::streamed(graph.weights()));
    logger->info("graph coords: \n{}", fmt::streamed(graph.coords()));

    tic;
    auto distance = gsp::pairwiseL2Distance(graph.coords());
    toc;
    logger->info("pairwiseL2Distance: \n{}", fmt::streamed(distance));

    tic;
    auto learned_weights = gsp::gaussianKernel(distance, 1);
    toc;
    logger->info("weight with gaussianKernel: \n{}", fmt::streamed(learned_weights));


    gsp::KnnDistance<double> knn;
    gsp::sparsematrix_t<double> sparse_distance, sparse_learned_weights;
    tic;
    sparse_distance = knn.setMetric(gsp::DistanceMetric::L2Distance)
            .setKFixed(10)
//            .setKPerDim(3)
            .setExcludeSelf(true)
            .setTriangularOnly(true)
            .compute(graph.coords());
    toc;
    logger->info("KNN L2Distance: \n{}", fmt::streamed(sparse_distance.toDense()));

    gsp::NanoflannAnnDistance<double> ann;
    tic;
    sparse_distance = ann.setMetric(gsp::DistanceMetric::L2Distance)
            .setKFixed(10)
            .setExcludeSelf(false)
            .setTriangularOnly(false)
            .compute(graph.coords());
    toc;
    logger->info("ANN L2Distance: \n{}", fmt::streamed(sparse_distance.toDense()));

    tic;
    sparse_learned_weights = gsp::gaussianKernel(sparse_distance, 1);
    toc;
    logger->info("weight with gaussianKernel: \n{}", fmt::streamed(sparse_learned_weights.toDense()));


    auto cos_sim = gsp::pairwiseCosineSimilarity(graph.coords());
    logger->info("pairwiseCosineSimilarity: \n{}", fmt::streamed(cos_sim));

    tic;
    auto chrd_dist = gsp::pairwiseChordalDistance(graph.coords());
    toc;
    logger->info("pairwiseChordalDistance: \n{}", fmt::streamed(chrd_dist));

    tic;
    sparse_distance = knn.setMetric(gsp::DistanceMetric::ChordalDistance)
            .setKFixed(10)
            .setExcludeSelf(false)
            .setTriangularOnly(false)
            .compute(graph.coords());
    toc;
    logger->info("KNN ChordalDistance: \n{}", fmt::streamed(sparse_distance.toDense()));

    tic;
    sparse_distance = ann.setMetric(gsp::DistanceMetric::ChordalDistance)
            .setKFixed(10)
            .setExcludeSelf(false)
            .setTriangularOnly(false)
            .compute(graph.coords());
    toc;
    logger->info("ANN ChordalDistance: \n{}", fmt::streamed(sparse_distance.toDense()));


    return 0;
}