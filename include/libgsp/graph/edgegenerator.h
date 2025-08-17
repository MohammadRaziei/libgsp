//
// Created by mohammad on 8/12/25.
//

#ifndef LIBGSP_EDGEGENERATOR_H
#define LIBGSP_EDGEGENERATOR_H
#pragma once

#include <Eigen/Dense>
#include <Eigen/SparseCore>

#include <vector>
#include <optional>

#include "libgsp/utils/types.h"   // defines densematrix, sparsematrix, and includes Eigen

namespace gsp {


template <class Matrix>
class Graph; // forward: must expose weights(), num_nodes, is_directed
class Edge; // forward: must expose row(), col(), weight()

// Single primary template; definitions specialized in .cpp
template <class Matrix>
class EdgeGenerator {
public:
    explicit EdgeGenerator(const Graph<Matrix>* graph);
    EdgeGenerator(const EdgeGenerator&) = delete;
    EdgeGenerator& operator=(const EdgeGenerator&) = delete;
    ~EdgeGenerator();

    void iter(types::elem_t<Matrix> thresh = 0.0);                       // reset internal cursor
    std::optional<Edge> next();        // next edge or std::nullopt
    std::vector<Edge> toVector();   // all edges as vector<Edge>

private:
    // common state (used by specializations)
    const Matrix*         _weights     = nullptr;
    int                   _num_nodes;
    types::elem_t<Matrix> _thresh;
    bool                  _is_directed = false;

    struct State;
    State* _state;
};

template <>
struct EdgeGenerator<densematrix>::State {
    State(const densematrix*) { reset(); }
    void reset() { _row = _col = 0; }
    uint32_t _row, _col;
};

template <>
struct EdgeGenerator<sparsematrix>::State {
    using InnerIt = sparsematrix::InnerIterator;

    explicit State(const sparsematrix* W) : W(W) { reset(); }

    void reset() {
        outer = 0;
        it.reset();
        // jump to first non-empty row
        advance_to_next_nonempty_row();
    }

    void advance_to_next_nonempty_row() {
        if (!W) return;
        const int outerSize = W->outerSize(); // == rows for RowMajor
        while (outer < outerSize) {
            it = std::make_unique<InnerIt>(*W, outer);
            if (*it) break;   // row has at least one nnz
            ++outer;          // try next row
        }
    }

    const sparsematrix* W = nullptr;
    uint32_t outer = 0;                          // current row
    std::unique_ptr<InnerIt> it;            // iterator within current row
};


} // namespace gsp



#endif  // LIBGSP_EDGEGENERATOR_H
