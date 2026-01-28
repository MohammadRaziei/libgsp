//
// Created by mohammad on 8/12/25.
//

#include "libgsp/VertexGraph.h"
#include <cassert>
#include <iostream>

#include "libgsp/utils/Logging.h"

uint32_t gsp::VertexGraph::num_graphs_ = 0;


gsp::VertexGraph::VertexGraph(uint32_t num_nodes) : num_nodes_(num_nodes), type_("VertexGraph"), id_(num_graphs_++),
    coords_() {
}

gsp::VertexGraph::VertexGraph(const gsp::VertexGraph *other) noexcept : num_nodes_(other->num_nodes_), type_(other->type_),
                                                                        coords_(other->coords_), names_(other->names_),
                                                                        id_(num_graphs_++) {
//    logger->trace("Copy constructor of VertexGraph is called!!");
}

gsp::VertexGraph::VertexGraph(gsp::VertexGraph &&other) noexcept : num_nodes_(other.num_nodes_), type_(other.type_),
    coords_(std::move(other.coords_)), names_(std::move(other.names_)), id_(other.id_) {
}

gsp::VertexGraph& gsp::VertexGraph::operator=(gsp::VertexGraph&& other) noexcept {
    if (this != &other) {
        num_nodes_ = other.num_nodes_;
        coords_ = std::move(other.coords_);
        names_ = std::move(other.names_);
        id_ = other.id_;
    }
    return *this;
}


void gsp::VertexGraph::setNames(const std::vector<std::string>& names) {
    assert(names.size() == num_nodes_ && "names size mismatch");
    this->names_ = names;
}
void gsp::VertexGraph::setCoords(const Eigen::MatrixXd& coords) {
    assert(coords.rows() == num_nodes_ && coords.cols() == 3 && "coords size mismatch");
    this->coords_ = coords;
}

void gsp::VertexGraph::clearNames() {
    this->names_.clear();
}
void gsp::VertexGraph::clearCoords() {
    this->coords_.setZero();
}
void gsp::VertexGraph::setCoords(const std::vector<gsp::Coord>& src) {
    assert(src.size() == num_nodes_ && "coords size mismatch");

    coords_.resize(num_nodes_, 3);
    for (uint32_t i = 0; i < num_nodes_; ++i) {
        coords_(i, 0) = src[i].x();
        coords_(i, 1) = src[i].y();
        coords_(i, 2) = src[i].z();
    }
}

gsp::VertexGraph::~VertexGraph() {
    --num_graphs_;
}

std::vector<gsp::Node> gsp::VertexGraph::nodes() const {
    std::vector<gsp::Node> vec;
    vec.reserve(num_nodes_);
    for (uint32_t i = 0; i < num_nodes_; ++i) {
        vec.emplace_back(i, coord(i), name(i));
    }
    return vec;
 }


 std::string gsp::VertexGraph::name(uint32_t idx) const{
     return (!names_.empty() && idx < names_.size()) ? names_[idx] :
     fmt::format(default_name_format, idx, idx, idx, idx, idx, idx, idx, idx);
 }

const gsp::Coord gsp::VertexGraph::coord(uint32_t idx) const {
    if (coords_.rows() > 0 && idx < static_cast<uint32_t>(coords_.rows())) {
        return {coords_(idx, 0), coords_(idx, 1), coords_(idx, 2), const_cast<VertexGraph*>(this), idx};
    } else {
        return {NAN, NAN, NAN, const_cast<VertexGraph*>(this), idx};
    }
}

gsp::Coord gsp::VertexGraph::coord(uint32_t idx) {
     if (coords_.rows() > 0 && idx < static_cast<uint32_t>(coords_.rows())) {
         return {coords_(idx, 0), coords_(idx, 1), coords_(idx, 2), this, idx};
     } else {
         return {NAN, NAN, NAN, this, idx};
     }
 }

void gsp::VertexGraph::setCoord(uint32_t idx, double x, double y, double z) {
    if (idx >= num_nodes_) {
        throw std::out_of_range("out of the range!");
    }
    if (coords_.rows() == 0) {
        coords_.resize(num_nodes_, 3);
        coords_.setZero();
    }
    coords_(idx, 0) = x;
    coords_(idx, 1) = y;
    coords_(idx, 2) = z;
}

void gsp::VertexGraph::setCoord(uint32_t idx, const Coord &coord) {
    setCoord(idx, coord.x(), coord.y(), coord.z());
}

void gsp::VertexGraph::setCoord(uint32_t row, uint32_t col, double value) {
    if (row >= num_nodes_ || col >= 3) {
        throw std::out_of_range("out of the range!");
    }
    if (coords_.rows() == 0) {
        coords_.resize(num_nodes_, 3);
        coords_.setZero();
    }
    coords_(row, col) = value;
}

gsp::VertexIterator gsp::VertexGraph::begin() {
    return VertexIterator(this, 0);
}

gsp::VertexIterator gsp::VertexGraph::end() {
    return VertexIterator(this, num_nodes_);
}

gsp::ConstVertexIterator gsp::VertexGraph::begin() const {
    return ConstVertexIterator(this, 0);
}

gsp::ConstVertexIterator gsp::VertexGraph::end() const {
    return ConstVertexIterator(this, num_nodes_);
}

gsp::ConstVertexIterator gsp::VertexGraph::cbegin() const {
    return ConstVertexIterator(this, 0);
}

gsp::ConstVertexIterator gsp::VertexGraph::cend() const {
    return ConstVertexIterator(this, num_nodes_);
}

std::reverse_iterator<gsp::VertexIterator> gsp::VertexGraph::rbegin() {
    return std::reverse_iterator<VertexIterator>(end());
}

std::reverse_iterator<gsp::VertexIterator> gsp::VertexGraph::rend() {
    return std::reverse_iterator<VertexIterator>(begin());
}

std::reverse_iterator<gsp::ConstVertexIterator> gsp::VertexGraph::rbegin() const {
    return std::reverse_iterator<ConstVertexIterator>(end());
}

std::reverse_iterator<gsp::ConstVertexIterator> gsp::VertexGraph::rend() const {
    return std::reverse_iterator<ConstVertexIterator>(begin());
}

std::reverse_iterator<gsp::ConstVertexIterator> gsp::VertexGraph::crbegin() const {
    return std::reverse_iterator<ConstVertexIterator>(cend());
}

std::reverse_iterator<gsp::ConstVertexIterator> gsp::VertexGraph::crend() const {
    return std::reverse_iterator<ConstVertexIterator>(cbegin());
}


gsp::Coord& gsp::Coord::operator=(const gsp::Coord& other) {
    if (&other == this) {
        return *this;
    }
    set(other, other.graph_ != nullptr);
    return *this;
}

gsp::Coord& gsp::Coord::operator=(gsp::Coord&& other) {
    if (&other == this) {
        return *this;
    }
    set(other, true);
    other.graph_ = nullptr;
    return *this;
}


void gsp::Coord::set(double x, double y, double z) {
    x_ = x; y_ = y; z_ = z;
    if (graph_) {
        graph_->setCoord(idx_, x, y, z);
    }
}

void gsp::Coord::setX(double x) {
    x_ = x;
    if (graph_) {
        graph_->setCoord(idx_, 0, x);
    }
}

void gsp::Coord::setY(double y) {
    y_ = y;
    if (graph_) {
        graph_->setCoord(idx_, 1, y);
    }
}

void gsp::Coord::setZ(double z) {
    z_ = z;
    if (graph_) {
        graph_->setCoord(idx_, 2, z);
    }
}

void gsp::Coord::set(const gsp::Coord& other, bool ownership) {
    if (&other == this) { return; }
    x_ = other.x_; y_ = other.y_; z_ = other.z_;
    if (ownership) {
        graph_ = other.graph_;
        idx_ = other.idx_;
    } else if (graph_) {
        graph_->setCoord(x_, y_, z_);
    }
}

gsp::Coord gsp::Coord::detach() const {
    return gsp::Coord(x_, y_, z_);
}

bool gsp::Coord::isDetached() const {
    return graph_ == nullptr;
}

gsp::Coord::Coord(const gsp::Coord& other) : x_(other.x_), y_(other.y_), z_(other.z_),
    graph_(other.graph_), idx_(other.idx_){
}

gsp::VertexGraph& gsp::VertexGraph::iadd(const gsp::VertexGraph& other) {
    if (numNodes() != other.numNodes()) {
        std::string msg = fmt::format("Number of nodes has mismatched! ({} != {})", numNodes(), other.numNodes());
        auto logger = gsp::logging::getLogger("VertexGraph");
        logger->error(msg);
        throw std::invalid_argument(msg);
    }
    if (names_.empty()) {
        setNames(other.names_);
    }
    if (coords_.rows() == 0) {
        coords_ = other.coords_;
    }
    return *this;
}

gsp::VertexGraph& gsp::VertexGraph::operator+=(const gsp::VertexGraph &other) {
    return iadd(other);
}

gsp::VertexGraph gsp::VertexGraph::add(const gsp::VertexGraph& other) const {
    return std::move(gsp::VertexGraph(this).iadd(other));
}

gsp::VertexGraph gsp::VertexGraph::operator+(const gsp::VertexGraph &other) const {
    return add(other);
}

gsp::VertexGraph gsp::VertexGraph::mul(const gsp::VertexGraph &other) const {
    const uint32_t num_nodes = num_nodes_ * other.num_nodes_;
    gsp::VertexGraph graph(num_nodes);

    graph.names_.resize(num_nodes);
    for (uint32_t i = 0; i < num_nodes_; ++i) {
        for (uint32_t j = 0; j < other.num_nodes_; ++j) {
            graph.names_[i * other.num_nodes_ + j] = name(i) + "," + other.name(j);
        }
    }

    if (coords_.rows() > 0 && other.coords_.rows() > 0) {
        graph.coords_.resize(num_nodes, 3);
        for (uint32_t i = 0; i < num_nodes_; ++i) {
            for (uint32_t j = 0; j < other.num_nodes_; ++j) {
                graph.coords_.row(i * other.num_nodes_ + j) = coords_.row(i) + other.coords_.row(j);
            }
        }
    }

    return graph;
}


gsp::VertexGraph gsp::VertexGraph::operator*(const gsp::VertexGraph &other) const {
    return mul(other);
}

