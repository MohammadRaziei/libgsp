//
// Created by mohammad on 8/12/25.
//

#ifndef LIBGSP_EDGEGENERATOR_H
#define LIBGSP_EDGEGENERATOR_H

#include "libgsp/utils/types.h"



#pragma once
#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <cstdint>
#include <type_traits>
#include <cmath>
#include <optional>

// must provide densematrix / sparsematrix aliases in this header (or included
// one)
#include "graph.h"
#include "libgsp/utils/types.h"   // defines densematrix, sparsematrix, and includes Eigen

namespace gsp {


template <class Matrix>
class Graph; // forward: must expose weights(), num_nodes, is_directed
class Edge; // forward: must expose row(), col(), weight()

// Single primary template; definitions specialized in .cpp
template <class Matrix>
class EdgeGenerator {
public:
    explicit EdgeGenerator(const gsp::Graph<Matrix>* graph, types::elem_t<Matrix> thresh = 0.0);
    EdgeGenerator(const EdgeGenerator&) = delete;
    EdgeGenerator& operator=(const EdgeGenerator&) = delete;
    ~EdgeGenerator();

    void iter();                       // reset internal cursor
    std::optional<Edge> next();        // next edge or std::nullopt
    std::vector<Edge> toVector();   // all edges as vector<Edge>

private:
    // common state (used by specializations)
    const Matrix*         _weights     = nullptr;
    int                   _num_nodes;
    types::elem_t<Matrix> _thresh;
    bool                  _is_directed = false;

    uint32_t _row, _col = 0;

    struct State;
    State* _state;
};

} // namespace gsp

template <>
struct gsp::EdgeGenerator<gsp::densematrix>::State {
    void reset() { row = col = 0; }
    uint32_t row, col = 0;
};

template <>
struct gsp::EdgeGenerator<gsp::sparsematrix>::State {

};



#endif  // LIBGSP_EDGEGENERATOR_H
