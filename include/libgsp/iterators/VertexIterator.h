#ifndef LIBGSP_VERTEXITERATOR_H
#define LIBGSP_VERTEXITERATOR_H

#include "../CommonTypes.h"
#include <iterator>
#include <cstddef>

// Forward declaration
namespace gsp {
    class VertexGraph;
} // namespace gsp

namespace gsp {

class VertexIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;  // Changed to bidirectional
    using value_type = Node;
    using difference_type = std::ptrdiff_t;
    using pointer = Node*;
    using reference = Node&;

    explicit VertexIterator(VertexGraph* graph, uint32_t index = 0)
        : graph_(graph), index_(index) {}

    // Dereference operator
    reference operator*() {
        if (index_ >= graph_->num_nodes) {
            throw std::out_of_range("VertexIterator out of range");
        }
        static thread_local Node node(index_, graph_->getCoord(index_), graph_->getName(index_));
        node.id = index_;
        return node;
    }

    // Pointer operator
    pointer operator->() {
        if (index_ >= graph_->num_nodes) {
            throw std::out_of_range("VertexIterator out of range");
        }
        static thread_local Node node(index_, graph_->getCoord(index_), graph_->getName(index_));
        node.id = index_;
        return &node;
    }

    // Pre-increment
    VertexIterator& operator++() {
        if (index_ < graph_->num_nodes) {
            ++index_;
        }
        return *this;
    }

    // Post-increment
    VertexIterator operator++(int) {
        VertexIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    // Pre-decrement (for bidirectional iterator)
    VertexIterator& operator--() {
        if (index_ > 0) {
            --index_;
        }
        return *this;
    }

    // Post-decrement (for bidirectional iterator)
    VertexIterator operator--(int) {
        VertexIterator tmp = *this;
        --(*this);
        return tmp;
    }

    // Equality comparison
    bool operator==(const VertexIterator& other) const {
        return graph_ == other.graph_ && index_ == other.index_;
    }

    // Inequality comparison
    bool operator!=(const VertexIterator& other) const {
        return !(*this == other);
    }

private:
    VertexGraph* graph_;
    uint32_t index_;
};

class ConstVertexIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;  // Changed to bidirectional
    using value_type = Node;
    using difference_type = std::ptrdiff_t;
    using pointer = const Node*;
    using reference = const Node&;

    explicit ConstVertexIterator(const VertexGraph* graph, uint32_t index = 0)
        : graph_(graph), index_(index) {}

    // Dereference operator
    reference operator*() const {
        if (index_ >= graph_->num_nodes) {
            throw std::out_of_range("ConstVertexIterator out of range");
        }
        static thread_local Node node(index_, graph_->getCoord(index_), graph_->getName(index_));
        node.id = index_;
        return node;
    }

    // Pointer operator
    pointer operator->() const {
        if (index_ >= graph_->num_nodes) {
            throw std::out_of_range("ConstVertexIterator out of range");
        }
        static thread_local Node node(index_, graph_->getCoord(index_), graph_->getName(index_));
        node.id = index_;
        return &node;
    }

    // Pre-increment
    ConstVertexIterator& operator++() {
        if (index_ < graph_->num_nodes) {
            ++index_;
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
        if (index_ > 0) {
            --index_;
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
    uint32_t index_;
};

} // namespace gsp

#endif // LIBGSP_VERTEXITERATOR_H