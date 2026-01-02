#ifndef LIBGSP_VERTEXGRAPH_ITERATORS_H
#define LIBGSP_VERTEXGRAPH_ITERATORS_H

#include "VertexGraph.h"
#include "iterators/VertexIterator.h"
#include <iterator>

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

inline std::reverse_iterator<ConstVertexIterator> VertexGraph::rbegin() const {
    return std::reverse_iterator<ConstVertexIterator>(end());
}

inline std::reverse_iterator<ConstVertexIterator> VertexGraph::rend() const {
    return std::reverse_iterator<ConstVertexIterator>(begin());
}

inline std::reverse_iterator<ConstVertexIterator> VertexGraph::crbegin() const {
    return std::reverse_iterator<ConstVertexIterator>(cend());
}

inline std::reverse_iterator<ConstVertexIterator> VertexGraph::crend() const {
    return std::reverse_iterator<ConstVertexIterator>(cbegin());
}

} // namespace gsp

#endif // LIBGSP_VERTEXGRAPH_ITERATORS_H