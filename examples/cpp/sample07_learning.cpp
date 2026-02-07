//
// Created by mohammad on 1/13/26.
//
#include <iostream>

#include "common.h"
#include "libgsp/utils/Logging.h"
#include "libgsp/Graph.h"
#include "libgsp/learning/Distance.h"


int main(int argc, char** argv) {
    gsp::logging::basicConfig(argc, argv, gsp::logging::level::trace);
    auto logger = gsp::logging::getLogger();

    logger->info("Sample 07 - graph learning examples");

    std::vector<gsp::Edge>  edges      = {{0, 0},{0,1, 3},{0,2},{1,2},{2,3}};
    std::vector<gsp::Coord> coords_vec = {{0,0}, {2,0}, {1,-1}, {3,-1}};

    gsp::DenseGraph graph(4);
    graph.setEdges(edges, false);
    graph.setCoords(coords_vec);

//    logger->info("graph W: \n{}", fmt::streamed(graph.weights()));
    logger->info("graph coords: \n{}", fmt::streamed(graph.coords()));

    auto distance = gsp::pairwiseDistance(graph.coords());
    logger->info("distances: \n{}", fmt::streamed(distance));


    gsp::KnnDistance<double> knn;
    knn.setMetric(gsp::DistanceMetric::L2)
            .setKFixed(10)
            .setExcludeSelf(true)
            .setTriangularOnly(false);

    knn.build(graph.coords());
    logger->info("knn distances: \n{}", fmt::streamed(knn.compute().toDense()));

    gsp::NanoflannAnnDistance<double> ann;
    ann.setMetric(gsp::DistanceMetric::L2)
            .setKFixed(10)
            .setExcludeSelf(true)
            .setTriangularOnly(true);

    ann.build(graph.coords());
    logger->info("nanoflann distances: \n{}", fmt::streamed(ann.compute().toDense()));


    return 0;
}