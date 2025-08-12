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
    explicit EdgeGenerator(const Graph<Matrix>* graph, types::elem_t<Matrix> thresh = 0.0);
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
    State(const sparsematrix* weights) :
        _rowPtr(weights->outerIndexPtr()),
        _colIdx(weights->innerIndexPtr()),
        _values(weights->valuePtr()) { reset(); }
    void reset() {
        _row = 0;
        _k = 0;
        _kend = 0;
    }
    void reset_row() {
        _k   = _rowPtr ? _rowPtr[_row] : 0;
        _kend= _rowPtr ? _rowPtr[_row+1] : 0;
    }
    uint32_t _row = 0;        // current row (outer)
    uint32_t _k   = 0;        // cursor inside row
    uint32_t _kend= 0;        // end cursor for this row
    const int*    _rowPtr  = nullptr; // size: n+1
    const int*    _colIdx  = nullptr; // size: nnz
    const double* _values  = nullptr; // size: nnz
};

} // namespace gsp



#endif  // LIBGSP_EDGEGENERATOR_H
