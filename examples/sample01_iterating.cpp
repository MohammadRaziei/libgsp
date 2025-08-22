//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/graph/graph.h"


int main() {
    std::vector<gsp::Edge>  edges      = {{0, 0},{0,1, 3},{0,2},{1,2},{2,3}};
    std::vector<gsp::Coord> coords_vec = {{0,0}, {2,0}, {1,-1}, {3,-1}};
    std::vector<double>     signal_vec = {-0.04, 0.31, 0.06, 0.39};

    const uint32_t num_nodes = 4;

    // Build graph
    gsp::DenseGraph graph(num_nodes);

    uint32_t count_nodes, num_edges;

    count_nodes = 0;
    graph.nodeIter();
    while (auto node = graph.nodeNext()) {
        printf("Node %i: %s @ (%.5g,%.5g,%.5g)\n", node->id, node->name.c_str(), node->coord.x, node->coord.y, node->coord.z);
        ++count_nodes;
    }
    printf("There is %i nodes\n", count_nodes);

    graph.setCoords(coords_vec);

    count_nodes = 0;
    graph.nodeIter();
    while (auto node = graph.nodeNext()) {
        printf("Node %i: %s @ (%.5g,%.5g,%.5g)\n", node->id, node->name.c_str(), node->coord.x, node->coord.y, node->coord.z);
        ++count_nodes;
    }
    printf("There is %i nodes\n", count_nodes);

    graph.setNames({"A", "B", "C", "D"});

    count_nodes = 0;
    graph.nodeIter();
    while (auto node = graph.nodeNext()) {
        printf("Node %i: %s @ (%.5g,%.5g,%.5g)\n", node->id, node->name.c_str(), node->coord.x, node->coord.y, node->coord.z);
        ++count_nodes;
    }
    printf("There is %i nodes\n", count_nodes);


    num_edges = 0;
    graph.edgeIter();
    while (auto edge = graph.edgeNext()) {
        printf("Edge: from[%i] to[%i] weight[%.5g]\n", edge->source, edge->target, edge->weight);
        ++num_edges;
    }
    printf("There is %i edges\n", num_edges);

    graph.setEdges(edges);           // fills graph.weights


    num_edges = 0;
    graph.edgeIter();
    while (auto edge = graph.edgeNext()) {
        printf("Edge: from[%i] to[%i] weight[%.5g]\n", edge->source, edge->target, edge->weight);
        ++num_edges;
    }
    printf("There is %i edges\n", num_edges);

    return 0;
}
