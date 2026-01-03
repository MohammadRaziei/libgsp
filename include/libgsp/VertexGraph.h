//
// Created by mohammad on 8/12/25.
//

#ifndef LIBGSP_VERTEXGRAPH_H
#define LIBGSP_VERTEXGRAPH_H

#include <utility>
#include <vector>
#include <optional>
#include <iterator>

#include <Eigen/Eigen>
#include "libgsp/CommonTypes.h"

// Forward declarations for iterators
namespace gsp {
    class VertexIterator;
    class ConstVertexIterator;
    class VertexGraph;
}  // namespace gsp


class gsp::VertexGraph {
public:
    using CoordMat = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;

    explicit VertexGraph(uint32_t);
    VertexGraph(const gsp::VertexGraph& other) = delete;
    void operator=(const gsp::VertexGraph& other) = delete;

    virtual ~VertexGraph();
    virtual void setCoords(const Eigen::MatrixXd& coords);
    virtual CoordMat coords();
    virtual void setCoords(const std::vector<gsp::Coord>&);
    virtual void setNames(const std::vector<std::string>&);
    virtual gsp::Coord getCoord(uint32_t idx) const;
    virtual std::string getName(uint32_t idx) const;

    virtual std::vector<gsp::Node> nodes() const;

    // Iterator methods
    ConstVertexIterator begin() const;
    ConstVertexIterator end() const;
    ConstVertexIterator cbegin() const;
    ConstVertexIterator cend() const;

    // Reverse iterator methods
    std::reverse_iterator<ConstVertexIterator> rbegin() const;
    std::reverse_iterator<ConstVertexIterator> rend() const;
    std::reverse_iterator<ConstVertexIterator> crbegin() const;
    std::reverse_iterator<ConstVertexIterator> crend() const;

public:
    const uint32_t num_nodes;
    std::vector<std::string> names;

   private:
    CoordMat _coords; // size fixed elsewhere to (num_nodes x 3)
    uint32_t _state_vertex;
};

#include "iterators/VertexIterator.h"
#include "VertexGraphIterators.h"

#endif  // LIBGSP_VERTEXGRAPH_H
