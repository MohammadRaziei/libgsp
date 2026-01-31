#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/optional.h>
#include <nanobind/eigen/dense.h>
#include <nanobind/eigen/sparse.h>

#include <string>
#include "libgsp/Graph.h"
#include "libgsp/Signal.h"
#include "libgsp/GraphSignal.h"
#include "libgsp/iterators/EdgeGenerator.h"
// #include "libgsp/io/File.h"

namespace nb = nanobind;
using namespace nb::literals;

// Template function for binding Graph class
template <typename Matrix>
void bind_graph(nb::module_ &m, const char *name) {
    using GraphType = gsp::Graph<Matrix>;
    
    nb::class_<GraphType, gsp::BaseGraph>(m, name)
        .def(nb::init<uint32_t, bool>(), "num_nodes"_a, "is_directed"_a = GSP_IS_DIRECTED_DEFAULT)
        .def(nb::init<gsp::VertexGraph&&>(), "other"_a)
        .def("setEdges", nb::overload_cast<const std::vector<gsp::Edge>&, bool>(&GraphType::setEdges), "edges"_a, "is_directed"_a = GSP_IS_DIRECTED_DEFAULT)
        .def("setEdges", nb::overload_cast<gsp::ConstEdgeGenerator&, bool>(&GraphType::setEdges), "generator"_a, "is_directed"_a = GSP_IS_DIRECTED_DEFAULT)
        .def("setWeights", nb::overload_cast<const Matrix&, bool>(&GraphType::setWeights), "matrix"_a, "is_directed"_a = GSP_IS_DIRECTED_DEFAULT)
        .def("setWeights", nb::overload_cast<const std::vector<gsp::Edge>&, bool>(&GraphType::setWeights), "edges"_a, "is_directed"_a = GSP_IS_DIRECTED_DEFAULT)
        .def("weights", &GraphType::weights, nb::rv_policy::reference_internal)
        .def("laplacian", &GraphType::laplacian, nb::rv_policy::reference_internal)
        .def("normalizedLaplacian", &GraphType::normalizedLaplacian, nb::rv_policy::reference_internal)
        .def("normalizedWeight", &GraphType::normalizedWeight, nb::rv_policy::reference_internal)
        .def("asymmetricNormalizedWeight", &GraphType::asymmetricNormalizedWeight, nb::rv_policy::reference_internal)
        .def("degrees", &GraphType::degrees, nb::rv_policy::reference_internal)
        .def("iterEdges", nb::overload_cast<double>(&GraphType::iterEdges, nb::const_), "thresh"_a = 0.0)
        .def("iterEdges", nb::overload_cast<double>(&GraphType::iterEdges), "thresh"_a = 0.0)
        .def("clone", &GraphType::clone)
        .def("toSparse", &GraphType::toSparse, "thresh"_a = 0.0, nb::rv_policy::take_ownership)
        .def("toDense", &GraphType::toDense, "thresh"_a = 0.0, nb::rv_policy::take_ownership)
        .def("invalidateCache", &GraphType::invalidateCache)
        .def("__add__", &GraphType::operator+, nb::rv_policy::take_ownership)
        .def("__iadd__", &GraphType::operator+=, nb::rv_policy::reference)
        .def("__mul__", &GraphType::operator*, nb::rv_policy::take_ownership)
        .def("add", &GraphType::add, "other"_a, nb::rv_policy::take_ownership)
        .def("mul", &GraphType::mul, "other"_a, nb::rv_policy::take_ownership)
        .def("kron", &GraphType::kron, "other"_a, nb::rv_policy::take_ownership);
}

// Template function for binding Signal class
template <typename T>
void bind_signal(nb::module_ &m, const char *name) {
    using SignalType = gsp::Signal<T>;
    
    nb::class_<SignalType>(m, name)
        .def("__init__", [](SignalType* self, int size) { new (self) SignalType(static_cast<size_t>(size)); }, "size"_a = 0)
        .def(nb::init<const typename SignalType::VectorT&>(), "vec"_a)
        .def(nb::init<const typename SignalType::VectorT&, const gsp::SignalMask&>(), "vec"_a, "mask"_a)
        .def("resize", &SignalType::resize)
        .def("size", &SignalType::size)
        .def("setMask", nb::overload_cast<gsp::SignalMask>(&SignalType::setMask))
        .def("setMask", nb::overload_cast<uint32_t, bool>(&SignalType::setMask))
        .def("mask", nb::overload_cast<>(&SignalType::mask, nb::const_))
        .def("mask", nb::overload_cast<uint32_t>(&SignalType::mask, nb::const_))
        .def("signal", nb::overload_cast<>(&SignalType::signal), nb::rv_policy::reference_internal)
        .def("signal", nb::overload_cast<uint32_t>(&SignalType::signal, nb::const_))
        .def("signal", nb::overload_cast<uint32_t>(&SignalType::signal), nb::rv_policy::reference)
        .def("set", &SignalType::set)
        .def("get", &SignalType::get)
        .def("vector", &SignalType::vector)
        .def("mul", nb::overload_cast<const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>&>(&SignalType::mul, nb::const_))
        .def("mul", nb::overload_cast<const Eigen::SparseMatrix<T>&>(&SignalType::mul, nb::const_))
        .def("imul", nb::overload_cast<const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>&>(&SignalType::imul))
        .def("imul", nb::overload_cast<const Eigen::SparseMatrix<T>&>(&SignalType::imul))
        .def("__mul__", nb::overload_cast<const SignalType&>(&SignalType::operator*, nb::const_))
        .def("__add__", nb::overload_cast<const SignalType&>(&SignalType::operator+, nb::const_))
        .def("__sub__", nb::overload_cast<const SignalType&>(&SignalType::operator-, nb::const_))
        .def("__imul__", nb::overload_cast<const SignalType&>(&SignalType::operator*=))
        .def("__iadd__", nb::overload_cast<const SignalType&>(&SignalType::operator+=))
        .def("__isub__", nb::overload_cast<const SignalType&>(&SignalType::operator-=))
        .def("__str__", [](const SignalType& s) { return static_cast<std::string>(s); })
        .def("compressed", &SignalType::compressed);
}

// Template function for binding GraphSignal class
template <typename Matrix, typename T>
void bind_graph_signal(nb::module_ &m, const char *name) {
    using GraphSignalType = gsp::GraphSignal<Matrix, T>;
    
    nb::class_<GraphSignalType>(m, name)
        .def(nb::init<gsp::Graph<Matrix>&, const gsp::Signal<T>&>(), "graph"_a, "signal"_a)
        .def("graph", [](GraphSignalType& self) -> gsp::Graph<Matrix>& { 
            return const_cast<gsp::Graph<Matrix>&>(self.graph()); 
        }, nb::rv_policy::reference)
        .def("signal", nb::overload_cast<>(&GraphSignalType::signal), nb::rv_policy::reference);
}

NB_MODULE(_graph, m) {
    m.doc() = "libgsp graph library";

    // Structs
    nb::class_<gsp::Coord>(m, "Coord")
        .def(nb::init<>())
        .def(nb::init<double, double, double>(), "x"_a, "y"_a, "z"_a = 0.0)
        .def_prop_rw("x", &gsp::Coord::x, &gsp::Coord::setX)
        .def_prop_rw("y", &gsp::Coord::y, &gsp::Coord::setY)
        .def_prop_rw("z", &gsp::Coord::z, &gsp::Coord::setZ)
        .def("set", [](gsp::Coord &c, double x, double y, double z = 0.0) { c.set(x, y, z); }, "x"_a, "y"_a, "z"_a = 0.0)
        .def("detach", &gsp::Coord::detach)
        .def("isDetached", &gsp::Coord::isDetached)
        .def("__repr__", [](const gsp::Coord &c) {
            return "Coord(x=" + std::to_string(c.x()) + ", y=" + std::to_string(c.y()) + ", z=" + std::to_string(c.z()) + ")";
        });

    nb::class_<gsp::Edge>(m, "Edge")
        .def(nb::init<uint32_t, uint32_t, double>(), "source"_a, "target"_a, "weight"_a = 1.0)
        .def_prop_ro("source", &gsp::Edge::source)
        .def_prop_ro("target", &gsp::Edge::target)
        .def_prop_rw("weight", &gsp::Edge::weight, &gsp::Edge::setWeight)
        .def("detach", &gsp::Edge::detach)
        .def("isDetached", &gsp::Edge::isDetached)
        .def("__repr__", [](const gsp::Edge &e) {
            return "Edge(source=" + std::to_string(e.source()) + ", target=" + std::to_string(e.target()) + ", weight=" + std::to_string(e.weight()) + ")";
        });

    nb::class_<gsp::Node>(m, "Node")
        .def(nb::init<uint32_t, const gsp::Coord&, const std::string&>(), "id"_a, "coord"_a, "name"_a)
        .def_rw("id", &gsp::Node::id)
        .def_rw("coord", &gsp::Node::coord)
        .def_rw("name", &gsp::Node::name)
        .def("__repr__", [](const gsp::Node &n) {
            return "Node(id=" + std::to_string(n.id) + ", coord=" + nb::cast<std::string>(nb::repr(nb::cast(n.coord))) + ", name=" + n.name + ")";
        });

    // Base classes
    nb::class_<gsp::VertexGraph>(m, "VertexGraph")
        .def(nb::init<uint32_t>(), "num_nodes"_a)
        .def("setCoords", nb::overload_cast<const gsp::VertexGraph::CoordMat&>(&gsp::VertexGraph::setCoords), "coords"_a)
        .def("setCoords", nb::overload_cast<const std::vector<gsp::Coord>&>(&gsp::VertexGraph::setCoords), "coords"_a)
        .def("setCoord", nb::overload_cast<uint32_t, const gsp::Coord&>(&gsp::VertexGraph::setCoord), "idx"_a, "coord"_a)
        .def("setCoord", nb::overload_cast<uint32_t, double, double, double>(&gsp::VertexGraph::setCoord), "idx"_a, "x"_a, "y"_a, "z"_a = 0.0)
        .def("coord", nb::overload_cast<uint32_t>(&gsp::VertexGraph::coord), "idx"_a)
        .def("coord", nb::overload_cast<uint32_t>(&gsp::VertexGraph::coord, nb::const_), "idx"_a)
        .def("setNames", &gsp::VertexGraph::setNames, "names"_a)
        .def("name", &gsp::VertexGraph::name, "idx"_a)
        .def("nodes", &gsp::VertexGraph::nodes)
        .def("clearNames", &gsp::VertexGraph::clearNames)
        .def("clearCoords", &gsp::VertexGraph::clearCoords)
        .def_prop_ro("num_nodes", &gsp::VertexGraph::numNodes)
        .def_prop_ro("names", &gsp::VertexGraph::names)
        .def_prop_ro("coords", &gsp::VertexGraph::coords)
        .def_prop_ro("type", &gsp::VertexGraph::type)
        .def("__add__", &gsp::VertexGraph::operator+, nb::rv_policy::take_ownership)
        .def("__iadd__", &gsp::VertexGraph::operator+=, nb::rv_policy::reference)
        .def("__mul__", &gsp::VertexGraph::operator*, nb::rv_policy::take_ownership)
        .def("add", &gsp::VertexGraph::add, "other"_a, nb::rv_policy::take_ownership)
        .def("mul", &gsp::VertexGraph::mul, "other"_a, nb::rv_policy::take_ownership);

    nb::class_<gsp::BaseGraph, gsp::VertexGraph>(m, "BaseGraph")
        .def("setEdges", &gsp::BaseGraph::setEdges, "edges"_a, "is_directed"_a = GSP_IS_DIRECTED_DEFAULT)
        .def("iterEdges", nb::overload_cast<double>(&gsp::BaseGraph::iterEdges, nb::const_), "thresh"_a = 0.0)
        .def("iterEdges", nb::overload_cast<double>(&gsp::BaseGraph::iterEdges), "thresh"_a = 0.0)
        .def("edges", &gsp::BaseGraph::edges, "thresh"_a = 0.0)
        .def("isDirected", &gsp::BaseGraph::isDirected)
        .def("setIsDirectedUnsafe", &gsp::BaseGraph::setIsDirectedUnsafe, "is_directed"_a);

    // Graph template specializations
    bind_graph<gsp::sparsematrix>(m, "SparseGraph");
    bind_graph<gsp::densematrix>(m, "DenseGraph");

    // EdgeGenerator classes - make them Python iterators
    nb::class_<gsp::EdgeGenerator>(m, "EdgeGenerator")
        .def("reset", &gsp::EdgeGenerator::reset)
        .def("next", &gsp::EdgeGenerator::next)
        .def("__iter__", [](gsp::EdgeGenerator &gen) -> gsp::EdgeGenerator& { 
            gen.reset(); 
            return gen; 
        })
        .def("__next__", [](gsp::EdgeGenerator &gen) -> std::optional<gsp::Edge> {
            auto edge = gen.next();
            if (!edge.has_value()) {
                throw nb::stop_iteration();
            }
            return edge;
        });

    nb::class_<gsp::ConstEdgeGenerator>(m, "ConstEdgeGenerator")
        .def("reset", &gsp::ConstEdgeGenerator::reset)
        .def("next", &gsp::ConstEdgeGenerator::next)
        .def("__iter__", [](gsp::ConstEdgeGenerator &gen) -> gsp::ConstEdgeGenerator& { 
            gen.reset(); 
            return gen; 
        })
        .def("__next__", [](gsp::ConstEdgeGenerator &gen) -> std::optional<const gsp::Edge> {
            auto edge = gen.next();
            if (!edge.has_value()) {
                throw nb::stop_iteration();
            }
            return edge;
        });

    // SignalMask
    nb::class_<gsp::SignalMask>(m, "SignalMask")
        .def(nb::init<uint32_t>(), "size"_a = 0)
        .def("resize", &gsp::SignalMask::resize)
        .def("set", &gsp::SignalMask::set, "idx"_a, "value"_a)
        .def("at", &gsp::SignalMask::at, "idx"_a)
        .def("size", &gsp::SignalMask::size)
        .def("nnz", &gsp::SignalMask::nnz)
        .def("__str__", &gsp::SignalMask::str);

    // Signal template specializations
    bind_signal<double>(m, "SignalDouble");
    bind_signal<float>(m, "SignalFloat");

    // GraphSignal template specializations - temporarily disabled due to const issues
    // bind_graph_signal<gsp::sparsematrix, double>(m, "GraphSignalSparseDouble");
    // bind_graph_signal<gsp::densematrix, double>(m, "GraphSignalDenseDouble");

    // IO functions
    // m.def("readFile", &gsp::io::readFile, "filename"_a);
    // m.def("writeFile", &gsp::io::writeFile, "filename"_a, "data"_a);
}
