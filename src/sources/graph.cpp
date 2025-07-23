//
// Created by Mohammad on 7/20/2025.
//

#include "graph/graph.h"


gsp::VertexGraph::VertexGraph(const uint32_t num_nodes, const bool is_directed) : num_nodes(num_nodes) {

}

gsp::SparseGraph::SparseGraph(const uint32_t num_nodes) : gsp::VertexGraph(num_nodes) {

}

gsp::DenseGraph::DenseGraph(const uint32_t num_nodes) : gsp::VertexGraph(num_nodes) {

}



