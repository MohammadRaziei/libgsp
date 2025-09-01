//
// Created by Mohammad on 8/23/2025.
//

#ifndef LIBGSP_FIGURE_H
#define LIBGSP_FIGURE_H
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace gsp {

// Forward declaration
class Axis;

// Layout configuration for the figure grid
struct FigureLayout {
    uint32_t rows = 1;
    uint32_t cols = 1;
    float gap_px = 12.0f;
    std::string css_class;
};

class Figure {
   public:
    // Constructor: creates a 1x1 figure by default
    explicit Figure(const std::string& title = "");

    // --- Title ---
    Figure& setTitle(const std::string& t);
    const std::string& title() const;

    // --- Grid setup ---
    Figure& setSubplot(uint32_t r, uint32_t c);
    Figure& setLayout(FigureLayout l);
    const FigureLayout& layout() const;

    uint32_t rows() const;
    uint32_t cols() const;
    uint32_t subplotCount() const;  // total number of subplots

    // --- Subplot access (by index or row/col) ---
    Axis& subplot(uint32_t i);
    Axis& subplot(uint32_t r, uint32_t c);

    // --- Rendering ---
    std::string render() const;

   private:
    // Resize the grid to r x c (minimum 1x1)
    Figure& resize(uint32_t r, uint32_t c);

    // Recreates the axes vector with new shared_ptr<Axis>
    void remake();

    // Converts (row, col) to flat index
    uint32_t idx(uint32_t r, uint32_t c) const;

   private:
    std::string _title;
    FigureLayout _layout;

    // Managed collection of subplots
    std::vector<std::shared_ptr<Axis>> _axes;

    // Render cache
    mutable bool _dirty = true;
    mutable std::string _rendered;
};

} // namespace gsp

#endif  // LIBGSP_FIGURE_H
