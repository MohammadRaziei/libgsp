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
#include "libgsp/EdgeGenerator.h"
#include "libgsp/io/File.h"

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(_graph, m) {
    m.doc() = "libgsp graph library";

    // Enums
    nb::enum_<gsp::ShiftType>(m, "ShiftType")
        .value("Weights", gsp::ShiftType::Weights)
        .value("Laplacian", gsp::ShiftType::Laplacian)
        .value("NormalizedWeights", gsp::ShiftType::NormalizedWeights)
        .value("NormalizedLaplacian", gsp::ShiftType::NormalizedLaplacian)
        .export_values();

    // Structs
    nb::class_<gsp::Coord>(m, "Coord")
        .def(nb::init<>())
        .def(nb::init<double, double, double>(), "x"_a, "y"_a, "z"_a = 0.0)
        .def_readwrite("x", &gsp::Coord::x)
        .def_readwrite("y", &gsp::Coord::y)
        .def_readwrite("z", &gsp::Coord::z)
        .def("__repr__", [](const gsp::Coord &c) {
            return "Coord(x=" + std::to_string(c.x) + ", y=" + std::to_string(c.y) + ", z=" + std::to_string(c.z) + ")";
        });

    nb::class_<gsp::Edge>(m, "Edge")
        .def(nb::init<uint32_t, uint32_t, double>(), "source"_a, "target"_a, "weight"_a = 1.0)
        .def_readwrite("source", &gsp::Edge::source)
        .def_readwrite("target", &gsp::Edge::target)
        .def_readwrite("weight", &gsp::Edge::weight)
        .def("__repr__", [](const gsp::Edge &e) {
            return "Edge(source=" + std::to_string(e.source) + ", target=" + std::to_string(e.target) + ", weight=" + std::to_string(e.weight) + ")";
        });

    nb::class_<gsp::Node>(m, "Node")
        .def(nb::init<uint32_t, const gsp::Coord&, const std::string&>(), "id"_a, "coord"_a, "name"_a)
        .def_readwrite("id", &gsp::Node::id)
        .def_readwrite("coord", &gsp::Node::coord)
        .def_readwrite("name", &gsp::Node::name)
        .def("__repr__", [](const gsp::Node &n) {
            return "Node(id=" + std::to_string(n.id) + ", coord=" + nb::cast<std::string>(nb::repr(nb::cast(n.coord))) + ", name=" + n.name + ")";
        });

    // Base classes
    nb::class_<gsp::VertexGraph>(m, "VertexGraph")
        .def("setCoords", nb::overload_cast<const gsp::VertexGraph::CoordMat&>(&gsp::VertexGraph::setCoords))
        .def("setCoords", nb::overload_cast<const std::vector<gsp::Coord>&>(&gsp::VertexGraph::setCoords))
        .def("coords", &gsp::VertexGraph::coords)
        .def("setNames", &gsp::VertexGraph::setNames)
        .def("getCoord", &gsp::VertexGraph::getCoord)
        .def("getName", &gsp::VertexGraph::getName)
        .def("nodeIter", &gsp::VertexGraph::nodeIter)
        .def("nodeNext", &gsp::VertexGraph::nodeNext)
        .def("nodes", &gsp::VertexGraph::nodes)
        .def_property_readonly("num_nodes", [](const gsp::VertexGraph& vg) { return vg.num_nodes; })
        .def_readonly("names", &gsp::VertexGraph::names);

    nb::class_<gsp::BaseGraph, gsp::VertexGraph>(m, "BaseGraph")
        .def("setEdges", &gsp::BaseGraph::setEdges)
        .def("edgeIter", &gsp::BaseGraph::edgeIter, "thresh"_a = 0.0)
        .def("edgeNext", &gsp::BaseGraph::edgeNext)
        .def("edges", &gsp::BaseGraph::edges);

    // Graph class template specializations
    nb::class_<gsp::SparseGraph, gsp::BaseGraph>(m, "SparseGraph")
        .def(nb::init<uint32_t>(), "num_nodes"_a)
        .def(nb::init<const gsp::VertexGraph&>(), "other"_a)
        .def("edgeIter", &gsp::SparseGraph::edgeIter, "thresh"_a = 0.0)
        .def("edgeNext", &gsp::SparseGraph::edgeNext)
        .def("edges", &gsp::SparseGraph::edges)
        .def("setEdges", &gsp::SparseGraph::setEdges)
        .def("setWeights", nb::overload_cast<const gsp::sparsematrix&, bool>(&gsp::SparseGraph::setWeights), "matrix"_a, "is_directed"_a = GSP_IS_DIRECTED_DEFAULT)
        .def("setWeights", nb::overload_cast<const std::vector<gsp::Edge>&, bool>(&gsp::SparseGraph::setWeights), "edges"_a, "is_directed"_a = GSP_IS_DIRECTED_DEFAULT)
        .def("isDirected", &gsp::SparseGraph::isDirected)
        .def("setIsDirectedUnsafe", &gsp::SparseGraph::setIsDirectedUnsafe)
        .def("weights", &gsp::SparseGraph::weights, nb::rv_policy::reference_internal)
        .def("laplacian", &gsp::SparseGraph::laplacian, nb::rv_policy::reference_internal)
        .def("normalizedLaplacian", &gsp::SparseGraph::normalizedLaplacian, nb::rv_policy::reference_internal)
        .def("degrees", &gsp::SparseGraph::degrees, nb::rv_policy::reference_internal)
        .def("invalidateCache", &gsp::SparseGraph::invalidateCache);

    nb::class_<gsp::DenseGraph, gsp::BaseGraph>(m, "DenseGraph")
        .def(nb::init<uint32_t>(), "num_nodes"_a)
        .def(nb::init<const gsp::VertexGraph&>(), "other"_a)
        .def("edgeIter", &gsp::DenseGraph::edgeIter, "thresh"_a = 0.0)
        .def("edgeNext", &gsp::DenseGraph::edgeNext)
        .def("edges", &gsp::DenseGraph::edges)
        .def("setEdges", &gsp::DenseGraph::setEdges)
        .def("setWeights", nb::overload_cast<const gsp::densematrix&, bool>(&gsp::DenseGraph::setWeights), "matrix"_a, "is_directed"_a = GSP_IS_DIRECTED_DEFAULT)
        .def("setWeights", nb::overload_cast<const std::vector<gsp::Edge>&, bool>(&gsp::DenseGraph::setWeights), "edges"_a, "is_directed"_a = GSP_IS_DIRECTED_DEFAULT)
        .def("isDirected", &gsp::DenseGraph::isDirected)
        .def("setIsDirectedUnsafe", &gsp::DenseGraph::setIsDirectedUnsafe)
        .def("weights", &gsp::DenseGraph::weights, nb::rv_policy::reference_internal)
        .def("laplacian", &gsp::DenseGraph::laplacian, nb::rv_policy::reference_internal)
        .def("normalizedLaplacian", &gsp::DenseGraph::normalizedLaplacian, nb::rv_policy::reference_internal)
        .def("degrees", &gsp::DenseGraph::degrees, nb::rv_policy::reference_internal)
        .def("invalidateCache", &gsp::DenseGraph::invalidateCache);

    // SignalMask
    nb::class_<gsp::SignalMask>(m, "SignalMask")
        .def(nb::init<uint32_t>(), "size"_a = 0)
        .def("resize", &gsp::SignalMask::resize)
        .def("set", &gsp::SignalMask::set)
        .def("at", &gsp::SignalMask::at)
        .def("setMask", &gsp::SignalMask::setMask)
        .def("setComplementMask", &gsp::SignalMask::setComplementMask)
        .def("__add__", &gsp::SignalMask::operator+)
        .def("__iadd__", &gsp::SignalMask::operator+=)
        .def("size", &gsp::SignalMask::size)
        .def("nnz", &gsp::SignalMask::nnz)
        .def("__str__", &gsp::SignalMask::str);

    // Signal template specializations
    nb::class_<gsp::Signal<double>>(m, "SignalDouble")
        .def(nb::init<int>(), "size"_a = 0)
        .def(nb::init<const gsp::Signal<double>::VectorT&>(), "vec"_a)
        .def(nb::init<const gsp::Signal<double>::VectorT&, const gsp::SignalMask&>(), "vec"_a, "mask"_a)
        .def("resize", &gsp::Signal<double>::resize)
        .def("size", &gsp::Signal<double>::size)
        .def("setMask", nb::overload_cast<gsp::SignalMask>(&gsp::Signal<double>::setMask))
        .def("setMask", nb::overload_cast<uint32_t, bool>(&gsp::Signal<double>::setMask))
        .def("setComplementMask", &gsp::Signal<double>::setComplementMask)
        .def("mask", nb::overload_cast<>(&gsp::Signal<double>::mask, nb::const_))
        .def("mask", nb::overload_cast<uint32_t>(&gsp::Signal<double>::mask, nb::const_))
        .def("signal", nb::overload_cast<>(&gsp::Signal<double>::signal), nb::rv_policy::reference_internal)
        .def("signal", nb::overload_cast<uint32_t>(&gsp::Signal<double>::signal, nb::const_))
        .def("signal", nb::overload_cast<uint32_t>(&gsp::Signal<double>::signal), nb::rv_policy::reference)
        .def("set", &gsp::Signal<double>::set)
        .def("get", &gsp::Signal<double>::get)
        .def("vector", &gsp::Signal<double>::vector)
        .def("mul", nb::overload_cast<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>&>(&gsp::Signal<double>::mul, nb::const_))
        .def("mul", nb::overload_cast<const Eigen::SparseMatrix<double>&>(&gsp::Signal<double>::mul, nb::const_))
        .def("imul", nb::overload_cast<const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>&>(&gsp::Signal<double>::imul))
        .def("imul", nb::overload_cast<const Eigen::SparseMatrix<double>&>(&gsp::Signal<double>::imul))
        .def("__mul__", nb::overload_cast<const gsp::Signal<double>&>(&gsp::Signal<double>::operator*, nb::const_))
        .def("__add__", nb::overload_cast<const gsp::Signal<double>&>(&gsp::Signal<double>::operator+, nb::const_))
        .def("__sub__", nb::overload_cast<const gsp::Signal<double>&>(&gsp::Signal<double>::operator-, nb::const_))
        .def("__div__", nb::overload_cast<const gsp::Signal<double>&>(&gsp::Signal<double>::operator/, nb::const_))
        .def("__imul__", nb::overload_cast<const gsp::Signal<double>&>(&gsp::Signal<double>::operator*=))
        .def("__iadd__", nb::overload_cast<const gsp::Signal<double>&>(&gsp::Signal<double>::operator+=))
        .def("__isub__", nb::overload_cast<const gsp::Signal<double>&>(&gsp::Signal<double>::operator-=))
        .def("__idiv__", nb::overload_cast<const gsp::Signal<double>&>(&gsp::Signal<double>::operator/=))
        .def("__str__", [](const gsp::Signal<double>& s) { return static_cast<std::string>(s); })
        .def("apply", &gsp::Signal<double>::apply<double(*)(double)>, "func"_a)
        .def("applyInplace", &gsp::Signal<double>::applyInplace<double(*)(double)>, "func"_a)
        .def("applyMask", &gsp::Signal<double>::applyMask)
        .def("compressed", &gsp::Signal<double>::compressed);

    nb::class_<gsp::Signal<float>>(m, "SignalFloat")
        .def(nb::init<int>(), "size"_a = 0)
        .def(nb::init<const gsp::Signal<float>::VectorT&>(), "vec"_a)
        .def(nb::init<const gsp::Signal<float>::VectorT&, const gsp::SignalMask&>(), "vec"_a, "mask"_a)
        .def("resize", &gsp::Signal<float>::resize)
        .def("size", &gsp::Signal<float>::size)
        .def("setMask", nb::overload_cast<gsp::SignalMask>(&gsp::Signal<float>::setMask))
        .def("setMask", nb::overload_cast<uint32_t, bool>(&gsp::Signal<float>::setMask))
        .def("setComplementMask", &gsp::Signal<float>::setComplementMask)
        .def("mask", nb::overload_cast<>(&gsp::Signal<float>::mask, nb::const_))
        .def("mask", nb::overload_cast<uint32_t>(&gsp::Signal<float>::mask, nb::const_))
        .def("signal", nb::overload_cast<>(&gsp::Signal<float>::signal), nb::rv_policy::reference_internal)
        .def("signal", nb::overload_cast<uint32_t>(&gsp::Signal<float>::signal, nb::const_))
        .def("signal", nb::overload_cast<uint32_t>(&gsp::Signal<float>::signal), nb::rv_policy::reference)
        .def("set", &gsp::Signal<float>::set)
        .def("get", &gsp::Signal<float>::get)
        .def("vector", &gsp::Signal<float>::vector)
        .def("mul", nb::overload_cast<const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>&>(&gsp::Signal<float>::mul, nb::const_))
        .def("mul", nb::overload_cast<const Eigen::SparseMatrix<float>&>(&gsp::Signal<float>::mul, nb::const_))
        .def("imul", nb::overload_cast<const Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>&>(&gsp::Signal<float>::imul))
        .def("imul", nb::overload_cast<const Eigen::SparseMatrix<float>&>(&gsp::Signal<float>::imul))
        .def("__mul__", nb::overload_cast<const gsp::Signal<float>&>(&gsp::Signal<float>::operator*, nb::const_))
        .def("__add__", nb::overload_cast<const gsp::Signal<float>&>(&gsp::Signal<float>::operator+, nb::const_))
        .def("__sub__", nb::overload_cast<const gsp::Signal<float>&>(&gsp::Signal<float>::operator-, nb::const_))
        .def("__div__", nb::overload_cast<const gsp::Signal<float>&>(&gsp::Signal<float>::operator/, nb::const_))
        .def("__imul__", nb::overload_cast<const gsp::Signal<float>&>(&gsp::Signal<float>::operator*=))
        .def("__iadd__", nb::overload_cast<const gsp::Signal<float>&>(&gsp::Signal<float>::operator+=))
        .def("__isub__", nb::overload_cast<const gsp::Signal<float>&>(&gsp::Signal<float>::operator-=))
        .def("__idiv__", nb::overload_cast<const gsp::Signal<float>&>(&gsp::Signal<float>::operator/=))
        .def("__str__", [](const gsp::Signal<float>& s) { return static_cast<std::string>(s); })
        .def("apply", &gsp::Signal<float>::apply<float(*)(float)>, "func"_a)
        .def("applyInplace", &gsp::Signal<float>::applyInplace<float(*)(float)>, "func"_a)
        .def("applyMask", &gsp::Signal<float>::applyMask)
        .def("compressed", &gsp::Signal<float>::compressed);

    // GraphSignal template specializations
    nb::class_<gsp::GraphSignal<gsp::sparsematrix, double>>(m, "GraphSignalSparseDouble")
        .def(nb::init<gsp::Graph<gsp::sparsematrix>&, const gsp::Signal<double>&>(), "graph"_a, "signal"_a)
        .def("graph", &gsp::GraphSignal<gsp::sparsematrix, double>::graph, nb::rv_policy::reference_internal)
        .def("signal", nb::overload_cast<>(&gsp::GraphSignal<gsp::sparsematrix, double>::signal), nb::rv_policy::reference_internal);

    nb::class_<gsp::GraphSignal<gsp::densematrix, double>>(m, "GraphSignalDenseDouble")
        .def(nb::init<gsp::Graph<gsp::densematrix>&, const gsp::Signal<double>&>(), "graph"_a, "signal"_a)
        .def("graph", &gsp::GraphSignal<gsp::densematrix, double>::graph, nb::rv_policy::reference_internal)
        .def("signal", nb::overload_cast<>(&gsp::GraphSignal<gsp::densematrix, double>::signal), nb::rv_policy::reference_internal);

    // EdgeGenerator template specializations
    nb::class_<gsp::EdgeGenerator<gsp::sparsematrix>>(m, "EdgeGeneratorSparse")
        .def(nb::init<const gsp::Graph<gsp::sparsematrix>*>(), "graph"_a)
        .def("iter", &gsp::EdgeGenerator<gsp::sparsematrix>::iter)
        .def("next", &gsp::EdgeGenerator<gsp::sparsematrix>::next)
        .def("toVector", &gsp::EdgeGenerator<gsp::sparsematrix>::toVector);

    nb::class_<gsp::EdgeGenerator<gsp::densematrix>>(m, "EdgeGeneratorDense")
        .def(nb::init<const gsp::Graph<gsp::densematrix>*>(), "graph"_a)
        .def("iter", &gsp::EdgeGenerator<gsp::densematrix>::iter)
        .def("next", &gsp::EdgeGenerator<gsp::densematrix>::next)
        .def("toVector", &gsp::EdgeGenerator<gsp::densematrix>::toVector);

    // IO functions
    m.def("readFile", &gsp::io::readFile, "filename"_a);
    m.def("writeFile", &gsp::io::writeFile, "filename"_a, "data"_a);
}