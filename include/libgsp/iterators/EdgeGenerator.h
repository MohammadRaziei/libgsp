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
    class ConstEdgeGenerator;
    class BaseStateEdgeGenerator;
    struct BaseMiniState;
} // namespace gsp

// Single primary template; definitions specialized in .cpp
class gsp::EdgeGenerator {
    friend class gsp::ConstEdgeGenerator;

public:
    explicit EdgeGenerator(std::shared_ptr<gsp::BaseStateEdgeGenerator> state);
    ~EdgeGenerator();

    void reset();
    std::optional<gsp::Edge> next();

private:
    std::shared_ptr<gsp::BaseStateEdgeGenerator> state_;
};

class gsp::ConstEdgeGenerator {
public:
    explicit ConstEdgeGenerator(std::shared_ptr<BaseStateEdgeGenerator> state);
    // Conversion from EdgeGenerator -> ConstEdgeGenerator
    ConstEdgeGenerator(const gsp::EdgeGenerator& other);

    ~ConstEdgeGenerator();

    void reset();
    const std::optional<const gsp::Edge> next(); // const-edge semantics via class type

    // Assignment from EdgeGenerator -> ConstEdgeGenerator
    ConstEdgeGenerator& operator=(const gsp::EdgeGenerator& other);

private:
    std::shared_ptr<gsp::BaseStateEdgeGenerator> state_;
};


class gsp::BaseStateEdgeGenerator {
public:
    BaseStateEdgeGenerator();
    virtual ~BaseStateEdgeGenerator();
    virtual void reset() = 0;
    virtual std::optional<gsp::Edge> next() = 0;
};

struct gsp::BaseMiniState {
    virtual ~BaseMiniState() = default;
    // Read the "current selected" weight (typically the last emitted edge).
    virtual double value() const = 0;
    // Write back to the "current selected" weight (typically the last emitted edge).
    virtual void setValue(double v) = 0;
};









#endif  // LIBGSP_EDGEGENERATOR_H
