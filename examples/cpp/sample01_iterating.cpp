//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/Graph.h"
#include "libgsp/utils/Logging.h"

uint32_t countNumNodes(gsp::BaseGraph& graph) {
    auto logger = gsp::logging::getLogger("countNumNodes");
    uint32_t count_nodes = 0;
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        logger->info("Node {}: {} @ ({:.5g},{:.5g},{:.5g})", it->id, it->name, it->coord.x, it->coord.y, it->coord.z);
        ++count_nodes;
    }
    logger->info("There is {} nodes", count_nodes);
    return count_nodes;
}

// Test function for new iterator functionality
void testIterators(gsp::BaseGraph& graph) {
    auto logger = gsp::logging::getLogger("testIterators");
    logger->info("Testing new iterator functionality:");

    // Test range-based for loop
    logger->info("Range-based for loop:");
    for (const auto& node : graph) {
        logger->info("  Node ID: {}, Name: {}, Coord: ({:.5g},{:.5g},{:.5g})",
                    node.id, node.name, node.coord.x, node.coord.y, node.coord.z);
    }

    // Test reverse iterators
    logger->info("Reverse iterator (rbegin to rend):");
    for (auto it = graph.rbegin(); it != graph.rend(); ++it) {
        logger->info("  Node ID: {}, Name: {}, Coord: ({:.5g},{:.5g},{:.5g})",
                    it->id, it->name, it->coord.x, it->coord.y, it->coord.z);
    }

    // Test const reverse iterators
    logger->info("Const reverse iterator (crbegin to crend):");
    const gsp::BaseGraph& const_graph = graph;
    for (auto it = const_graph.crbegin(); it != const_graph.crend(); ++it) {
        logger->info("  Node ID: {}, Name: {}, Coord: ({:.5g},{:.5g},{:.5g})",
                    it->id, it->name, it->coord.x, it->coord.y, it->coord.z);
    }
}

uint32_t countNumEdges(gsp::BaseGraph& graph) {
    auto logger = gsp::logging::getLogger("countNumEdges");

    uint32_t num_edges = 0;
    graph.edgeIter();
    while (auto edge = graph.edgeNext()) {
        logger->info("Edge: from[{}] to[{}] weight[{:.5g}]", edge->source, edge->target, edge->weight);
        ++num_edges;
    }
    logger->info("There is {} edges", num_edges);
    return num_edges;
}


int main() {
    gsp::logging::basicConfig(gsp::logging::level::debug);
    auto logger = gsp::logging::getLoggerByPath(__FILE__);

    logger->info("Sample 01: Iterating over nodes and edges");

    std::vector<gsp::Edge>  edges      = {{0, 0},{0,1, 3},{0,2},{1,2},{2,3}};
    std::vector<gsp::Coord> coords_vec = {{0,0}, {2,0}, {1,-1}, {3,-1}};

    const uint32_t num_nodes = 4;

    // Build graph
    gsp::DenseGraph graph(num_nodes);

    countNumNodes(graph);

    graph.setCoords(coords_vec);

    countNumNodes(graph);

    graph.setNames({"A", "B", "C", "D"});

    countNumNodes(graph);


    countNumEdges(graph);


    graph.setEdges(edges);           // fills graph.weights


    countNumEdges(graph);

    // Test the new iterator functionality
    testIterators(graph);

    return 0;
}
