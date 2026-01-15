//
// Created by mohammad on 1/13/26.
//
#include <iostream>

#include "common.h"
#include "libgsp/utils/Logging.h"
#include "libgsp/Graph.h"
#include "libgsp/spectral/PageRank.h"

auto logger = gsp::logging::getLoggerByPath(__FILE__);

template <class PageRankAlg, typename Matrix>
void testPagerankAlgorithms(gsp::Graph<Matrix>& graph, double p) {
    stic; PageRankAlg pr(graph, p); stoc;
    auto elapsed1 = tictoc;

    stic; auto pagerank = pr.run(); stoc;
    auto elapsed2 = tictoc;

    logger->info("method: {}, graph type: {}, elapsed time: {} ({}+{}), page rank: \n{}",
                 pr.method(), graph.type(), elapsed1 + elapsed2, elapsed1, elapsed2,
                 fmt::streamed(pagerank.transpose()));
}

int main() {
    logger->info("Sample 06 - spectral examples");

    std::vector<gsp::Edge>  edges      = {{0, 0},{0,1, 3},{0,2},{1,2},{2,3}};
    std::vector<gsp::Coord> coords_vec = {{0,0}, {2,0}, {1,-1}, {3,-1}};

    gsp::DenseGraph dense_graph(4);
    dense_graph.setEdges(edges, false);
    dense_graph.setCoords(coords_vec);

    logger->info("graph W: \n{}", fmt::streamed(dense_graph.weights()));
    gsp::SparseGraph sparse_graph = dense_graph.toSparse();

    testPagerankAlgorithms<gsp::PageRankBase<gsp::densematrix>>(dense_graph, 1);
    testPagerankAlgorithms<gsp::PageRankBase<gsp::sparsematrix>>(sparse_graph, 1);

    testPagerankAlgorithms<gsp::PageRankSpectra<gsp::densematrix>>(dense_graph, 1);
    testPagerankAlgorithms<gsp::PageRankSpectra<gsp::sparsematrix>>(sparse_graph, 1);

    testPagerankAlgorithms<gsp::PageRankPowerMethod<gsp::densematrix>>(dense_graph, 1);
    testPagerankAlgorithms<gsp::PageRankPowerMethod<gsp::sparsematrix>>(sparse_graph, 1);


    return 0;
}