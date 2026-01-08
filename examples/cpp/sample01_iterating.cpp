//
// Created by Mohammad on 8/22/2025.
//

#include "libgsp/Graph.h"
#include "libgsp/utils/Logging.h"

uint32_t countNumNodes(gsp::BaseGraph& graph) {
    auto logger = gsp::logging::getLogger("countNumNodes");
    uint32_t count_nodes = 0;
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        logger->info("Node {}: {} @ ({:.5g},{:.5g},{:.5g})", it->id, it->name, it->coord.x(), it->coord.y(), it->coord.z());
        ++count_nodes;
    }
    logger->info("There is {} nodes", count_nodes);
    return count_nodes;
}

uint32_t countNumEdges(gsp::BaseGraph& graph) {
    auto logger = gsp::logging::getLogger("countNumEdges");

    uint32_t num_edges = 0;

    // Use the new iterEdges API instead of the old edgeIter/edgeNext
    if (auto dense_graph = dynamic_cast<gsp::Graph<gsp::densematrix>*>(&graph)) {
        auto edge_gen = dense_graph->iterEdges();
        while (auto edge = edge_gen.next()) {
            logger->info("Edge: from[{}] to[{}] weight[{:.5g}]", edge->source, edge->target, edge->weight);
            ++num_edges;
        }
    } else if (auto sparse_graph = dynamic_cast<gsp::Graph<gsp::sparsematrix>*>(&graph)) {
        auto edge_gen = sparse_graph->iterEdges();
        while (auto edge = edge_gen.next()) {
            logger->info("Edge: from[{}] to[{}] weight[{:.5g}]", edge->source, edge->target, edge->weight);
            ++num_edges;
        }
    } else {
        // Fallback: use the edges() method
        auto all_edges = graph.edges();
        for (const auto& edge : all_edges) {
            logger->info("Edge: from[{}] to[{}] weight[{:.5g}]", edge.source, edge.target, edge.weight);
            ++num_edges;
        }
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
    logger->info("Testing new iterator functionality:");

    // Test range-based for loop
    logger->info("Range-based for loop:");
    for (const auto& node : graph) {
        logger->info("  Node ID: {}, Name: {}, Coord: ({:.5g},{:.5g},{:.5g})",
                     node.id, node.name, node.coord.x(), node.coord.y(), node.coord.z());
    }

    // Test non-const begin/end
    logger->info("Non-const begin to end:");
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        auto& coord = it->coord;
        coord.setCoord(coord.x()* 2, coord.y()*2, coord.z()+1);
        logger->info("  Node ID: {}, Name: {}, Coord: ({:.5g},{:.5g},{:.5g})",
                    it->id, it->name, it->coord.x(), it->coord.y(), it->coord.z());
    }

    // Test const begin/end
    logger->info("Const begin to end:");
    const gsp::BaseGraph& const_graph = graph;
    for (auto it = const_graph.begin(); it != const_graph.end(); ++it) {
        // it->coord.setCoord(2, 2, 2); // ERROR
        logger->info("  Node ID: {}, Name: {}, Coord: ({:.5g},{:.5g},{:.5g})",
                    it->id, it->name, it->coord.x(), it->coord.y(), it->coord.z());
    }

    // Test cbegin/cend
    logger->info("cbegin to cend:");
    for (auto it = const_graph.cbegin(); it != const_graph.cend(); ++it) {
        logger->info("  Node ID: {}, Name: {}, Coord: ({:.5g},{:.5g},{:.5g})",
                    it->id, it->name, it->coord.x(), it->coord.y(), it->coord.z());
    }

    // Test new EdgeGenerator functionality
    logger->info("Testing EdgeGenerator with iterEdges:");

    {
        const auto dense_graph = graph.toDense();
        auto edge_gen = dense_graph->iterEdges();
        logger->info("Dense graph edges via iterEdges:");
        int edge_count = 0;
        while (auto edge = edge_gen.next()) {
            logger->info("  Edge {}: from {} to {} with weight {:.5g}",
                        edge_count++, edge->source, edge->target, edge->weight);
        }
    }

    // Test sparse graph edges
    {
        const auto sparse_graph = graph.toSparse();
        auto edge_gen = sparse_graph->iterEdges();
        logger->info("Sparse graph edges via iterEdges:");
        int edge_count = 0;
        while (auto edge = edge_gen.next()) {
            logger->info("  Edge {}: from {} to {} with weight {:.5g}",
                        edge_count++, edge->source, edge->target, edge->weight);
        }
    }

    // Test edge iteration with threshold
    {
        const auto dense_graph = graph.toDense();
        auto edge_gen = dense_graph->iterEdges(0.5); // threshold of 0.5
        logger->info("Dense graph edges with threshold 0.5:");
        int edge_count = 0;
        while (auto edge = edge_gen.next()) {
            logger->info("  Edge {}: from {} to {} with weight {:.5g}",
                        edge_count++, edge->source, edge->target, edge->weight);
        }
    }

    // Test reset functionality
    {
        const auto dense_graph = graph.toDense();

        auto edge_gen = dense_graph->iterEdges();
        logger->info("Testing reset functionality:");

        // Get first edge
        if (auto edge = edge_gen.next()) {
            logger->info("  First edge: from {} to {} with weight {:.5g}",
                        edge->source, edge->target, edge->weight);
        }

        // Reset the generator
        edge_gen.reset();

        // Get first edge again
        if (auto edge = edge_gen.next()) {
            logger->info("  First edge after reset: from {} to {} with weight {:.5g}",
                        edge->source, edge->target, edge->weight);
        }
    }

    // Test enhanced VertexIterator functionality
    logger->info("Testing enhanced VertexIterator functionality:");

    // Test arithmetic operations
    logger->info("Testing iterator arithmetic:");
    auto begin_it = graph.begin();
    auto end_it = graph.end();
    logger->info("Graph has {} nodes", end_it - begin_it);

    // Test += operator
    auto it = graph.begin();
    logger->info("First node: ID={}, Name={}", it->id, it->name);
    it += 2;
    logger->info("After += 2: ID={}, Name={}", it->id, it->name);

    // Test -= operator
    it -= 1;
    logger->info("After -= 1: ID={}, Name={}", it->id, it->name);

    // Test + operator
    auto it_plus_2 = graph.begin() + 2;
    logger->info("Iterator + 2: ID={}, Name={}", it_plus_2->id, it_plus_2->name);

    // Test - operator
    auto it_minus_1 = it_plus_2 - 1;
    logger->info("Iterator - 1: ID={}, Name={}", it_minus_1->id, it_minus_1->name);

    // Test subscript operator
    logger->info("Testing subscript operator:");
    auto start_it = graph.begin();
    logger->info("start_it[0]: ID={}, Name={}", start_it[0].id, start_it[0].name);
    logger->info("start_it[1]: ID={}, Name={}", start_it[1].id, start_it[1].name);
    logger->info("start_it[2]: ID={}, Name={}", start_it[2].id, start_it[2].name);

    // Test comparison operators
    logger->info("Testing comparison operators:");
    auto it1 = graph.begin();
    auto it2 = graph.begin() + 1;
    logger->info("it1 < it2: {}", it1 < it2);
    logger->info("it1 <= it2: {}", it1 <= it2);
    logger->info("it1 > it2: {}", it1 > it2);
    logger->info("it1 >= it1: {}", it1 >= it1);

    // Test random access iteration
    logger->info("Testing random access iteration:");
    for (int i = 0; i < (graph.end() - graph.begin()); ++i) {
        auto node_it = graph.begin() + i;
        logger->info("  Node at index {}: ID={}, Name={}", i, node_it->id, node_it->name);
    }

    // Test const iterator functionality
    logger->info("Testing const iterator functionality:");
    auto const_begin = const_graph.begin();
    auto const_end = const_graph.end();
    logger->info("Const iterator arithmetic: distance = {}", const_end - const_begin);

    auto const_it = const_graph.begin() + 1;
    logger->info("Const iterator at position 1: ID={}, Name={}", const_it->id, const_it->name);

    // Test iterator subtraction
    logger->info("Testing iterator subtraction:");
    auto pos1 = graph.begin() + 1;
    auto pos3 = graph.begin() + 3;
    logger->info("Distance between position 3 and 1: {}", pos3 - pos1);

    logger->info("All enhanced iterator functionality tests completed!");

    return 0;
}
