//
// Created by mohammad on 9/11/25.
//

#ifndef LIBGSP_PLOTSVG_H
#define LIBGSP_PLOTSVG_H

#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>

#include <Eigen/Core>

#include "BaseGraph.h"
#include "GraphSignal.h"

namespace gsp {

class GraphSvg {
public:
    explicit GraphSvg(const gsp::BaseGraph& graph,
                     std::optional<gsp::Signal<double>> signal = std::nullopt);
    ~GraphSvg();

    GraphSvg(const GraphSvg&) = delete;
    GraphSvg& operator=(const GraphSvg&) = delete;
    GraphSvg(GraphSvg&&) noexcept;
    GraphSvg& operator=(GraphSvg&&) noexcept;

    // ---- Configuration ----
    GraphSvg& setTitle(const std::string& title);
    GraphSvg& setLibGspVersion(const std::string& version);
    GraphSvg& setNodeSpaceScale(double v);
    GraphSvg& setSignalScale(double v);
    GraphSvg& setStyleOverride(const std::string& css);
    GraphSvg& setSvgCss(const std::string& css);
    GraphSvg& setSvgBackground(const std::string& css);

    // ---- Signal API ----
    GraphSvg& setSignal(const Eigen::VectorXd& s);
    GraphSvg& addSignal(uint32_t idx, double value);

    GraphSvg& loadConfig(const std::string& file_path);
    GraphSvg& loadsConfig(const std::string& toml_text);
    GraphSvg& dumpConfig(const std::string& file_path) const;
    std::string dumpsConfig() const;


    // ---- Render ----
    const std::string& render() const;
    const std::string& svg() const;
    void save(const std::string& filename) const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;  //
};

} // namespace gsp


#endif  // LIBGSP_PLOTSVG_H
