//
// Created by Mohammad on 8/23/2025.
//

#ifndef LIBGSP_FIGURE_H
#define LIBGSP_FIGURE_H

#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <sstream>

class Axis;

struct FigureLayout {
    uint32_t rows = 1;
    uint32_t cols = 1;
    float    gap_px  = 12.0f;
    std::string css_class;
};

/**
 * Figure is a lightweight HTML layout container of Axis cells.
 * It does NOT own Axis objects; it stores non-owning pointers.
 * Callers must ensure Axis lifetime outlives the Figure::render() call.
 */
class Figure {
   public:
    explicit Figure(std::string title = "")
        : _title(std::move(title)) { resize(1, 1); }

    // --- basics ---
    Figure& setTitle(const std::string& t) { _title = t; _dirty = true; return *this; }
    const std::string& title() const { return _title; }

    Figure& setSubplot(uint32_t r, uint32_t c) { return resize(r, c); }
    Figure& setLayout(FigureLayout l) { _layout = std::move(l); remake(); _dirty = true; return *this; }
    const FigureLayout& layout() const { return _layout; }

    uint32_t rows() const { return _layout.rows; }
    uint32_t cols() const { return _layout.cols; }
    uint32_t size() const { return static_cast<uint32_t>(_axes.size()); }

    // --- subplot setters/getters (pointer-based, non-owning) ---
    // Set all subplots at once; size must be rows*cols
    Figure& setSubplots(const std::vector<Axis*>& axes);

    // Get the underlying pointer grid (row-major)
    const std::vector<Axis*>& getSubplots() const { return _axes; }

    // Set a single cell (row, col)
    Figure& setSubplot(uint32_t r, uint32_t c, Axis* ax);

    // Get by (row, col)
    Axis*       getSubplot(uint32_t r, uint32_t c)       { return _axes.at(idx(r, c)); }
    const Axis* getSubplot(uint32_t r, uint32_t c) const { return _axes.at(idx(r, c)); }

    // 1-arg accessors by flat index
    Axis*       getSubplot(uint32_t i)       { return _axes.at(i); }
    const Axis* getSubplot(uint32_t i) const { return _axes.at(i); }

    // Aliases
    Axis*       at(uint32_t i)       { return _axes.at(i); }
    const Axis* at(uint32_t i) const { return _axes.at(i); }

    // --- rendering with cache ---
    // Builds a full, self-contained HTML page (no external deps).
    // Caches the result until Figure is mutated.
    std::string render() const;

   private:
    Figure& resize(uint32_t r, uint32_t c) {
        if (r == 0 || c == 0) throw std::invalid_argument("rows/cols must be >= 1");
        _layout.rows = r; _layout.cols = c; remake(); _dirty = true; return *this;
    }

    void remake() {
        _axes.assign(static_cast<size_t>(_layout.rows) * _layout.cols, nullptr);
        _dirty = true;
    }

    uint32_t idx(uint32_t r, uint32_t c) const {
        if (r >= _layout.rows || c >= _layout.cols) throw std::out_of_range("subplot index");
        return r * _layout.cols + c;
    }

   private:
    std::string   _title;
    FigureLayout  _layout{};

    // Row-major grid of non-owning pointers
    std::vector<Axis*> _axes;

    // Render cache
    mutable bool        _dirty    = true;
    mutable std::string _rendered;
};

#endif  // LIBGSP_FIGURE_H
