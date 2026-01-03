#ifndef LIBGSP_VERTEXGRAPH_ITERATORS_H
#define LIBGSP_VERTEXGRAPH_ITERATORS_H

#include "VertexGraph.h"
#include "iterators/VertexIterator.h"

namespace gsp {

inline ConstVertexIterator VertexGraph::begin() const {
    return ConstVertexIterator(this, 0);
}

inline ConstVertexIterator VertexGraph::end() const {
    return ConstVertexIterator(this, num_nodes);
}

inline ConstVertexIterator VertexGraph::cbegin() const {
    return ConstVertexIterator(this, 0);
}

inline ConstVertexIterator VertexGraph::cend() const {
    return ConstVertexIterator(this, num_nodes);
}

} // namespace gsp

#endif // LIBGSP_VERTEXGRAPH_ITERATORS_H