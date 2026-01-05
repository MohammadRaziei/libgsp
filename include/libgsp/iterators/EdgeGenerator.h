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

#include "libgsp/utils/Types.h"  // defines densematrix, sparsematrix, and includes Eigen

namespace gsp {


class Edge; // forward: must expose source, target, weight

// Single primary template; definitions specialized in .cpp
template <class Matrix>
class EdgeGenerator {
public:
    EdgeGenerator(const Matrix* weights, int num_nodes, bool is_directed, types::elem_t<Matrix> thresh);
    EdgeGenerator(const EdgeGenerator&) = delete;
    EdgeGenerator& operator=(const EdgeGenerator&) = delete;
    ~EdgeGenerator();

    void reset();  // reset internal cursor (keeps current threshold)
    std::optional<Edge> next();        // next edge or std::nullopt

private:
    // common state (used by specializations)
    const Matrix*         _weights     = nullptr;
    int                   _num_nodes;
    types::elem_t<Matrix> _thresh;
    bool                  _is_directed;

    struct State;
    std::unique_ptr<State> _state;  // Using PIMPL pattern for state
};

} // namespace gsp



#endif  // LIBGSP_EDGEGENERATOR_H
