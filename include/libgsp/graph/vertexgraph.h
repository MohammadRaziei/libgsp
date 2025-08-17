//
// Created by mohammad on 8/12/25.
//

#ifndef LIBGSP_VERTEXGRAPH_H
#define LIBGSP_VERTEXGRAPH_H

#include <vector>
#include <Eigen/Eigen>


namespace gsp {
    struct Coord;


    class VertexGraph;
}  // namespace gsp

struct gsp::Coord {
    Coord(double x, double y, double z = 0) : x(x), y(y), z(z) {}
    double x, y, z;
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

public:
    const uint32_t num_nodes;
    std::vector<std::string> names;

   private:
    CoordMat _coords; // size fixed elsewhere to (num_nodes x 3)
};


#endif  // LIBGSP_VERTEXGRAPH_H
