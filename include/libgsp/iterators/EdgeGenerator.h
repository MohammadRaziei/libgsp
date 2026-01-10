//
// Created by mohammad on 8/12/25.
//

#ifndef LIBGSP_EDGEGENERATOR_H
#define LIBGSP_EDGEGENERATOR_H
#pragma once

#include <vector>
#include <optional>
#include <memory>

#include <libgsp/BaseGraph.h>

namespace gsp {
    class EdgeGenerator;
//    class ConstEdgeGenerator;
    class BaseStateEdgeGenerator;
} // namespace gsp

// Single primary template; definitions specialized in .cpp
class gsp::EdgeGenerator {
public:
    explicit EdgeGenerator(std::shared_ptr<BaseStateEdgeGenerator> state);
    ~EdgeGenerator();

    void reset();
    std::optional<gsp::Edge> next();

private:
    std::shared_ptr<BaseStateEdgeGenerator> state_;
};

//class gsp::ConstEdgeGenerator {
//public:
//    explicit ConstEdgeGenerator(std::shared_ptr<BaseStateEdgeGenerator> state);
//    ~ConstEdgeGenerator();
//
//    void reset();
//    const std::optional<gsp::Edge> next();
//
//private:
//    std::shared_ptr<BaseStateEdgeGenerator> state_;
//};


class gsp::BaseStateEdgeGenerator {
public:
    BaseStateEdgeGenerator();
    virtual ~BaseStateEdgeGenerator();
    virtual void reset() = 0;
    virtual std::optional<gsp::Edge> next() = 0;
    virtual void setWeight(double weight) = 0;
    virtual std::shared_ptr<BaseStateEdgeGenerator> clone() const = 0;
};










#endif  // LIBGSP_EDGEGENERATOR_H
