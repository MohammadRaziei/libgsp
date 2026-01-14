//
// Created by mohammad on 1/13/26.
//
#include <iostream>

#include "libgsp/utils/Logging.h"
#include "libgsp/Graph.h"
#include "libgsp/spectral/PageRank.h"

int main() {
    auto logger = gsp::logging::getLoggerByPath(__FILE__);
    logger->info("Sample 06 - spectral examples");


    std::vector<gsp::Edge>  edges      = {{0, 0},{0,1, 3},{0,2},{1,2},{2,3}};
    std::vector<gsp::Coord> coords_vec = {{0,0}, {2,0}, {1,-1}, {3,-1}};

    gsp::DenseGraph graph(4);
    graph.setEdges(edges, false);
    graph.setCoords(coords_vec);

    logger->info("graph W: \n{}", fmt::streamed(graph.weights()));

    auto pagerank_spectra = gsp::PageRankSpectra(graph, 1).run();
    logger->info("page rank: \n{}", fmt::streamed(pagerank_spectra));



    return 0;
}