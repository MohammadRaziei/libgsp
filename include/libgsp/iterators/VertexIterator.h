#ifndef LIBGSP_VERTEXITERATOR_H
#define LIBGSP_VERTEXITERATOR_H
#pragma once

#include <iterator>
#include <cstddef>

#include "libgsp/VertexGraph.h"


namespace gsp {
namespace detail {

template <class GraphT>
class TemplateVertexIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = Node;
    using difference_type   = std::ptrdiff_t;

    static constexpr bool kIsConstGraph = std::is_const_v<GraphT>;

    using pointer   = std::conditional_t<kIsConstGraph, const Node*, Node*>;
    using reference = std::conditional_t<kIsConstGraph, const Node&, Node&>;

    TemplateVertexIterator() = default;

    explicit TemplateVertexIterator(GraphT* graph, int64_t index = 0)
        : graph_(graph), index_(index) {
        updateCurrent();
    }


    // Dereference operator (non-const) - only for non-const GraphT
    template <class T = GraphT, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    reference operator*() {
        ensureValidForDeref();
        return *current_; // Node&
    }

// Dereference operator (const) - works for both, returns reference type
    reference operator*() const {
        ensureValidForDeref();
        return *current_; // const Node& when GraphT is const
    }

// Pointer operator (non-const) - only for non-const GraphT
    template <class T = GraphT, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    pointer operator->() {
        ensureValidForDeref();
        return std::addressof(*current_); // Node*
    }

// Pointer operator (const)
    pointer operator->() const {
        ensureValidForDeref();
        return std::addressof(*current_); // const Node* when GraphT is const
    }

    // Pre-increment
    TemplateVertexIterator& operator++() {
        ++index_;
        updateCurrent();
        return *this;
    }

    // Post-increment
    TemplateVertexIterator operator++(int) {
        TemplateVertexIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    // Pre-decrement
    TemplateVertexIterator& operator--() {
        --index_;
        updateCurrent();
        return *this;
    }

    // Post-decrement
    TemplateVertexIterator operator--(int) {
        TemplateVertexIterator tmp = *this;
        --(*this);
        return tmp;
    }

    // Arithmetic operators
    TemplateVertexIterator& operator+=(difference_type n) {
        index_ += static_cast<int64_t>(n);
        updateCurrent();
        return *this;
    }

    TemplateVertexIterator& operator-=(difference_type n) {
        index_ -= static_cast<int64_t>(n);
        updateCurrent();
        return *this;
    }

    TemplateVertexIterator operator+(difference_type n) const {
        TemplateVertexIterator tmp = *this;
        tmp += n;
        return tmp;
    }

    TemplateVertexIterator operator-(difference_type n) const {
        TemplateVertexIterator tmp = *this;
        tmp -= n;
        return tmp;
    }

    // Iterator difference
    difference_type operator-(const TemplateVertexIterator& other) const {
        ensureSameGraph(other);
        return static_cast<difference_type>(index_ - other.index_);
    }

    // Subscript operator (returns a value, like your original)
    value_type operator[](difference_type n) const {
        ensureGraph();
        const int64_t new_index = index_ + static_cast<int64_t>(n);
        const int64_t nn = numNodesSigned();

        if (new_index < 0 || new_index >= nn) {
            throw std::out_of_range("TemplateVertexIterator subscript out of range");
        }

        const uint32_t ui = static_cast<uint32_t>(new_index);
        return value_type(ui, graph_->coord(ui), graph_->name(ui));
    }

    // Comparisons
    bool operator==(const TemplateVertexIterator& other) const {
        return graph_ == other.graph_ && index_ == other.index_;
    }
    bool operator!=(const TemplateVertexIterator& other) const { return !(*this == other); }

    bool operator<(const TemplateVertexIterator& other) const {
        ensureSameGraph(other);
        return index_ < other.index_;
    }
    bool operator<=(const TemplateVertexIterator& other) const {
        ensureSameGraph(other);
        return index_ <= other.index_;
    }
    bool operator>(const TemplateVertexIterator& other) const {
        ensureSameGraph(other);
        return index_ > other.index_;
    }
    bool operator>=(const TemplateVertexIterator& other) const {
        ensureSameGraph(other);
        return index_ >= other.index_;
    }

    // Conversion: non-const iterator -> const iterator
    template <class T = GraphT, std::enable_if_t<!std::is_const_v<T>, int> = 0>
    operator TemplateVertexIterator<const std::remove_const_t<GraphT>>() const {
        using ConstGraph = const std::remove_const_t<GraphT>;
        return TemplateVertexIterator<ConstGraph>(graph_, static_cast<uint32_t>(index_));
    }

    const GraphT* graph() const { return graph_; }
    int64_t index() const { return index_; }

protected:
    GraphT* graph_ = nullptr;
    int64_t index_ = 0;
    std::optional<value_type> current_;

    void ensureGraph() const {
        if (!graph_) {
            throw std::logic_error("TemplateVertexIterator has null graph pointer");
        }
    }

    int64_t numNodesSigned() const {
        // Convert once to signed to do safe range checks with index_
        return static_cast<int64_t>(graph_->numNodes());
    }

    void ensureSameGraph(const TemplateVertexIterator& other) const {
        if (graph_ != other.graph_) {
            throw std::invalid_argument("Cannot operate on iterators from different graphs");
        }
    }

    void ensureValidForDeref() const {
        ensureGraph();
        if (!current_.has_value()) {
            throw std::out_of_range("TemplateVertexIterator out of range");
        }
    }

    void updateCurrent() {
        if (!graph_) {
            current_.reset();
            return;
        }

        const int64_t nn = numNodesSigned();
        if (index_ < 0 || index_ >= nn) {
            current_.reset();
            return;
        }

        const uint32_t ui = static_cast<uint32_t>(index_);
        if (!current_) {
            current_.emplace(ui, graph_->coord(ui), graph_->name(ui));
        } else {
            current_->id    = ui;
            current_->coord = graph_->coord(ui);
            current_->name  = graph_->name(ui);
        }
    }
};

} // namespace detail


class VertexIterator : public detail::TemplateVertexIterator<VertexGraph> {
    using Base = detail::TemplateVertexIterator<VertexGraph>;
public:
    using Base::Base; // Inherit constructors
};

class ConstVertexIterator : public detail::TemplateVertexIterator<const VertexGraph> {
    using Base = detail::TemplateVertexIterator<const VertexGraph>;
public:
    using Base::Base;

    // Conversion ctor: VertexIterator -> ConstVertexIterator
    ConstVertexIterator(const gsp::VertexIterator& other)
            : Base(other.graph(), other.index()) {
    }

    // Assignment: VertexIterator -> ConstVertexIterator
    ConstVertexIterator& operator=(const gsp::VertexIterator& other) {
        this->graph_ = other.graph();
        this->index_ = other.index();
        this->updateCurrent();
        return *this;
    }
};

} // namespace gsp

#endif // LIBGSP_VERTEXITERATOR_H