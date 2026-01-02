#ifndef LIBGSP_COMMONTYPES_H
#define LIBGSP_COMMONTYPES_H

#include <string>
#include <cstdint>

namespace gsp {

struct Coord {
    Coord() = default;
    Coord(double x, double y, double z = 0) : x(x), y(y), z(z) {}
    double x, y, z;
};

struct Node {
    Node(uint32_t id, const Coord& coord, const std::string& name):
          id(id), coord(coord), name(name) {}
    uint32_t id;
    gsp::Coord coord;
    std::string name;
};

} // namespace gsp

#endif // LIBGSP_COMMONTYPES_H