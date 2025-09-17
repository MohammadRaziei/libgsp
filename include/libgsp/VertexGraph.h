//
// Created by mohammad on 8/12/25.
//

#ifndef LIBGSP_VERTEXGRAPH_H
#define LIBGSP_VERTEXGRAPH_H

#include <utility>
#include <vector>
#include <optional>

#include <Eigen/Eigen>

namespace gsp {
    struct Coord;
    struct Node;


    class VertexGraph;
}  // namespace gsp

struct gsp::Coord {
    Coord() = default;
    Coord(double x, double y, double z = 0) : x(x), y(y), z(z) {}
    double x, y, z;
};

struct gsp::Node {
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
    void operator=(const gsp::VertexGraph& other) = delete;

    virtual ~VertexGraph();
    virtual void setCoords(const Eigen::MatrixXd& coords);
    virtual CoordMat coords();
    virtual void setCoords(const std::vector<gsp::Coord>&);
    virtual void setNames(const std::vector<std::string>&);
    virtual gsp::Coord getCoord(uint32_t idx) const;
    virtual std::string getName(uint32_t idx) const;

    virtual void nodeIter();
    virtual std::optional<gsp::Node> nodeNext();
    virtual std::vector<gsp::Node> nodes() const;

public:
    const uint32_t num_nodes;
    std::vector<std::string> names;

   private:
    CoordMat _coords; // size fixed elsewhere to (num_nodes x 3)
    uint32_t _state_vertex;
};


#endif  // LIBGSP_VERTEXGRAPH_H
