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
    using iterator_category = std::random_access_iterator_tag;  // Changed to random access
    using value_type = Node;
    using difference_type = std::ptrdiff_t;
    using pointer = const Node*;
    using reference = const Node&;

    explicit ConstVertexIterator(const VertexGraph* graph, uint32_t index = 0)
        : graph_(graph), index_(index), current_(index, graph->coord(index), graph->name(index)) {
    }

    // Dereference operator
    reference operator*() const {
        if (index_ < 0 || static_cast<uint32_t>(index_) >= graph_->numNodes()) {
            throw std::out_of_range("ConstVertexIterator out of range");
        }
        return current_;
    }

    // Pointer operator
    pointer operator->() const {
        if (index_ < 0 || static_cast<uint32_t>(index_) >= graph_->numNodes()) {
            throw std::out_of_range("ConstVertexIterator out of range");
        }
        return &current_;
    }

    // Pre-increment
    ConstVertexIterator& operator++() {
        if (++index_ < static_cast<int32_t>(graph_->numNodes())) {
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

    // Arithmetic operators
    ConstVertexIterator& operator+=(difference_type n) {
        index_ += n;
        if (index_ >= 0 && index_ < graph_->numNodes()) {
            updateCurrent();
        }
        return *this;
    }

    ConstVertexIterator& operator-=(difference_type n) {
        index_ -= n;
        if (index_ >= 0 && index_ < graph_->numNodes()) {
            updateCurrent();
        }
        return *this;
    }

    ConstVertexIterator operator+(difference_type n) const {
        ConstVertexIterator tmp = *this;
        tmp += n;
        return tmp;
    }

    ConstVertexIterator operator-(difference_type n) const {
        ConstVertexIterator tmp = *this;
        tmp -= n;
        return tmp;
    }

    difference_type operator-(const ConstVertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot subtract iterators from different graphs");
        }
        return index_ - other.index_;
    }

    // Subscript operator
    value_type operator[](difference_type n) const {
        // Create a temporary iterator at the target position and return its value
        difference_type new_index = index_ + n;
        if (new_index < 0 || new_index >= static_cast<difference_type>(graph_->numNodes())) {
            throw std::out_of_range("ConstVertexIterator subscript out of range");
        }
        // Create a temporary node at the target position
        return value_type(new_index, graph_->coord(new_index), graph_->name(new_index));
    }

    // Comparison operators
    bool operator==(const ConstVertexIterator& other) const {
        return graph_ == other.graph_ && index_ == other.index_;
    }

    bool operator!=(const ConstVertexIterator& other) const {
        return !(*this == other);
    }

    bool operator<(const ConstVertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot compare iterators from different graphs");
        }
        return index_ < other.index_;
    }

    bool operator<=(const ConstVertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot compare iterators from different graphs");
        }
        return index_ <= other.index_;
    }

    bool operator>(const ConstVertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot compare iterators from different graphs");
        }
        return index_ > other.index_;
    }

    bool operator>=(const ConstVertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot compare iterators from different graphs");
        }
        return index_ >= other.index_;
    }

private:
    const VertexGraph* graph_;
    int64_t index_;
    value_type current_;

    void updateCurrent() {
        if (index_ >= 0 && index_ < graph_->numNodes()) {
            current_.id = index_;
            current_.coord = std::move(graph_->coord(index_));
            current_.name = std::move(graph_->name(index_));
        }
    }
};

class VertexIterator {
public:
    using iterator_category = std::random_access_iterator_tag;  // Changed to random access
    using value_type = Node;
    using difference_type = std::ptrdiff_t;
    using pointer = Node*;
    using reference = Node&;

    explicit VertexIterator(VertexGraph* graph, uint32_t index = 0)
        : graph_(graph), index_(index), current_(index, graph->coord(index), graph->name(index)) {
    }

    // Dereference operator
    reference operator*() const {
        if (index_ < 0 || static_cast<uint32_t>(index_) >= graph_->numNodes()) {
            throw std::out_of_range("VertexIterator out of range");
        }
        return const_cast<reference>(current_);
    }

    // Pointer operator
    pointer operator->() const {
        if (index_ < 0 || static_cast<uint32_t>(index_) >= graph_->numNodes()) {
            throw std::out_of_range("VertexIterator out of range");
        }
        return const_cast<pointer>(&current_);
    }

    // Pre-increment
    VertexIterator& operator++() {
        if (++index_ < static_cast<int32_t>(graph_->numNodes())) {
            updateCurrent();
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
        if (--index_ >= 0) {
            updateCurrent();
        }
        return *this;
    }

    // Post-decrement (for bidirectional iterator)
    VertexIterator operator--(int) {
        VertexIterator tmp = *this;
        --(*this);
        return tmp;
    }

    // Arithmetic operators
    VertexIterator& operator+=(difference_type n) {
        index_ += n;
        if (index_ >= 0 && index_ < graph_->numNodes()) {
            updateCurrent();
        }
        return *this;
    }

    VertexIterator& operator-=(difference_type n) {
        index_ -= n;
        if (index_ >= 0 && index_ < graph_->numNodes()) {
            updateCurrent();
        }
        return *this;
    }

    VertexIterator operator+(difference_type n) const {
        VertexIterator tmp = *this;
        tmp += n;
        return tmp;
    }

    VertexIterator operator-(difference_type n) const {
        VertexIterator tmp = *this;
        tmp -= n;
        return tmp;
    }

    difference_type operator-(const VertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot subtract iterators from different graphs");
        }
        return index_ - other.index_;
    }

    // Subscript operator
    value_type operator[](difference_type n) const {
        // Create a temporary iterator at the target position and return its value
        difference_type new_index = index_ + n;
        if (new_index < 0 || new_index >= static_cast<difference_type>(graph_->numNodes())) {
            throw std::out_of_range("VertexIterator subscript out of range");
        }
        // Create a temporary node at the target position
        return value_type(new_index, graph_->coord(new_index), graph_->name(new_index));
    }

    // Comparison operators
    bool operator==(const VertexIterator& other) const {
        return graph_ == other.graph_ && index_ == other.index_;
    }

    bool operator!=(const VertexIterator& other) const {
        return !(*this == other);
    }

    bool operator<(const VertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot compare iterators from different graphs");
        }
        return index_ < other.index_;
    }

    bool operator<=(const VertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot compare iterators from different graphs");
        }
        return index_ <= other.index_;
    }

    bool operator>(const VertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot compare iterators from different graphs");
        }
        return index_ > other.index_;
    }

    bool operator>=(const VertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot compare iterators from different graphs");
        }
        return index_ >= other.index_;
    }

    // Conversion from VertexIterator to ConstVertexIterator
    operator ConstVertexIterator() const {
        return ConstVertexIterator(graph_, static_cast<uint32_t>(index_));
    }

private:
    VertexGraph* graph_;
    int64_t index_;
    mutable value_type current_;  // mutable to allow updates in const methods

    void updateCurrent() const {
        if (index_ >= 0 && index_ < graph_->numNodes()) {
            current_.id = static_cast<uint32_t>(index_);
            current_.coord = std::move(graph_->coord(static_cast<uint32_t>(index_)));
            current_.name = std::move(graph_->name(static_cast<uint32_t>(index_)));
        }
    }
};

} // namespace gsp

#endif // LIBGSP_VERTEXITERATOR_H