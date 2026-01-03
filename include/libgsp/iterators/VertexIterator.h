#ifndef LIBGSP_VERTEXITERATOR_H
#define LIBGSP_VERTEXITERATOR_H
#pragma once

#include <iterator>
#include <cstddef>

#include "libgsp/VertexGraph.h"

// Forward declaration
namespace gsp {
    class VertexGraph;
} // namespace gsp

namespace gsp {


class ConstVertexIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;  // Changed to bidirectional
    using value_type = Node;
    using difference_type = std::ptrdiff_t;
    using pointer = const Node*;
    using reference = const Node&;

    explicit ConstVertexIterator(const VertexGraph* graph, uint32_t index = 0)
        : graph_(graph), index_(index), current_(index, graph->coord(index), graph->name(index)) {
    }

    // Dereference operator
    reference operator*() const {
        if (index_ < 0 || static_cast<uint32_t>(index_) >= graph_->num_nodes) {
            throw std::out_of_range("ConstVertexIterator out of range");
        }
        return current_;
    }

    // Pointer operator
    pointer operator->() const {
        if (index_ < 0 || static_cast<uint32_t>(index_) >= graph_->num_nodes) {
            throw std::out_of_range("ConstVertexIterator out of range");
        }
        return &current_;
    }

    // Pre-increment
    ConstVertexIterator& operator++() {
        if (++index_ < static_cast<int32_t>(graph_->num_nodes)) {
            updateCurrent();
        }
        return *this;
    }

    // Post-increment
    ConstVertexIterator operator++(int) {
        ConstVertexIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    // Pre-decrement (for bidirectional iterator)
    ConstVertexIterator& operator--() {
        if (--index_ >= 0) {
            updateCurrent();
        }
        return *this;
    }

    // Post-decrement (for bidirectional iterator)
    ConstVertexIterator operator--(int) {
        ConstVertexIterator tmp = *this;
        --(*this);
        return tmp;
    }

    // Equality comparison
    bool operator==(const ConstVertexIterator& other) const {
        return graph_ == other.graph_ && index_ == other.index_;
    }

    // Inequality comparison
    bool operator!=(const ConstVertexIterator& other) const {
        return !(*this == other);
    }

private:
    const VertexGraph* graph_;
    int64_t index_;
    value_type current_;

    void updateCurrent() {
        if (index_ >= 0 && index_ < graph_->num_nodes) {
            current_.id = index_;
            current_.coord = graph_->coord(index_);
            current_.name = graph_->name(index_);
        }
    }
};

} // namespace gsp

#endif // LIBGSP_VERTEXITERATOR_H