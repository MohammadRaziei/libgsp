#ifndef LIBGSP_VERTEXGRAPH_ITERATORS_H
#define LIBGSP_VERTEXGRAPH_ITERATORS_H

#include "VertexGraph.h"
#include "iterators/VertexIterator.h"
#include <iterator>

namespace gsp {

// Inline implementations of iterator methods
inline VertexIterator VertexGraph::begin() {
    return VertexIterator(this, 0);
}

inline VertexIterator VertexGraph::end() {
    return VertexIterator(this, num_nodes);
}

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

// Inline implementations of reverse iterator methods
inline std::reverse_iterator<VertexIterator> VertexGraph::rbegin() {
    return std::reverse_iterator<VertexIterator>(end());
}

inline std::reverse_iterator<VertexIterator> VertexGraph::rend() {
    return std::reverse_iterator<VertexIterator>(begin());
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