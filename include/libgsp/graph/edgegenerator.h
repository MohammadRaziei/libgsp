//
// Created by mohammad on 8/12/25.
//

#ifndef LIBGSP_EDGEGENERATOR_H
#define LIBGSP_EDGEGENERATOR_H

#include "libgsp/graph/graph.h"
#include "libgsp/utils/types.h"



#pragma once
#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <cstdint>
#include <type_traits>
#include <cmath>
#include <optional>

// must provide densematrix / sparsematrix aliases in this header (or included one)
#include "libgsp/utils/types.h"   // defines densematrix, sparsematrix, and includes Eigen

namespace gsp {


template <class Matrix>
class Graph; // forward: must expose weights(), num_nodes, is_directed

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
    const gsp::Graph<Matrix>* G_ = nullptr;
    const Matrix*             W_ = nullptr;
    int    n_        = 0;
    types::elem_t<Matrix> thr_      = 0.0;
    bool   directed_ = false;

    // dense iteration
    int i_ = 0, j_ = 0;

    // sparse iteration (RowMajor storage)
    int outer_ = 0, k_ = 0;
    const int*    outerPtr_  = nullptr;
    const int*    innerIdx_  = nullptr;
    const types::elem_t<Matrix>* values_    = nullptr;
};

} // namespace gsp





#endif  // LIBGSP_EDGEGENERATOR_H
