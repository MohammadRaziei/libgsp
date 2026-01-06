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

// Forward declarations for iterators
namespace gsp {
    class VertexIterator;
    class ConstVertexIterator;
    class VertexGraph;
    class Coord;
    struct Node;
}  // namespace gsp

class gsp::Coord {
public:
    Coord() : x_(0), y_(0), z_(0), graph_(nullptr), idx_(0u) {}
    Coord(double x, double y, double z = 0, VertexGraph* graph = nullptr, uint32_t idx = 0u) :
        x_(x), y_(y), z_(z), graph_(graph), idx_(idx) {}
    double x() const {return x_;}
    double y() const {return y_;}
    double z() const {return z_;}
    void setCoord(double x, double y, double z = 0.);
    void setX(double x);
    void setY(double y);
    void setZ(double z);

protected:
    double x_, y_, z_;
    uint32_t idx_;
    VertexGraph* graph_;
};

struct gsp::Node {
    Node() = default;
    Node(uint32_t id, const Coord& coord, const std::string& name):
            id(id), coord(coord), name(name) {}
    uint32_t id;
    gsp::Coord coord;
    std::string name;
};



class gsp::VertexGraph {
public:
    using CoordMat = Eigen::Matrix<double, Eigen::Dynamic, 3, Eigen::RowMajor>;

    explicit VertexGraph(uint32_t);
    VertexGraph(const gsp::VertexGraph& other) = delete;
    VertexGraph(gsp::VertexGraph&& other) noexcept;
    explicit VertexGraph(const gsp::VertexGraph* other) noexcept;

    gsp::VertexGraph& operator=(const gsp::VertexGraph& other) = delete;
    gsp::VertexGraph& operator=(gsp::VertexGraph&& other) noexcept;

    virtual ~VertexGraph();
    virtual CoordMat coords() const;
    virtual void setCoords(const Eigen::MatrixXd& coords);
    virtual void setCoords(const std::vector<gsp::Coord>& coords);
    virtual void setCoord(uint32_t idx, const gsp::Coord& coord);
    virtual void setCoord(uint32_t row, uint32_t col, double value);
    virtual void setCoord(uint32_t idx, double x, double y, double z);
    virtual void setNames(const std::vector<std::string>&);
    virtual gsp::Coord coord(uint32_t idx);
    virtual const gsp::Coord coord(uint32_t idx) const;
    virtual const std::string name(uint32_t idx) const;

    virtual std::vector<gsp::Node> nodes() const;

    // Iterator methods
    VertexIterator begin();
    VertexIterator end();
    ConstVertexIterator begin() const;
    ConstVertexIterator end() const;
    ConstVertexIterator cbegin() const;
    ConstVertexIterator cend() const;

    // Reverse iterator methods
    std::reverse_iterator<VertexIterator> rbegin();
    std::reverse_iterator<VertexIterator> rend();
    std::reverse_iterator<ConstVertexIterator> rbegin() const;
    std::reverse_iterator<ConstVertexIterator> rend() const;
    std::reverse_iterator<ConstVertexIterator> crbegin() const;
    std::reverse_iterator<ConstVertexIterator> crend() const;

public:
    uint32_t num_nodes;
    std::vector<std::string> names;

   private:
    CoordMat _coords; // size fixed elsewhere to (num_nodes x 3)
};

#include "libgsp/iterators/VertexIterator.h"


#endif  // LIBGSP_VERTEXGRAPH_H
