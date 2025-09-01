//
// Created by Mohammad on 8/23/2025.
//

#include "libgsp/plotting/figure.h"
#include "libgsp/io/file.h"
#include "libgsp/plotting/axis.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

#include <mustache.hpp>

namespace fs = std::filesystem;
using namespace kainjow;

namespace gsp {

// --- Static: Get template or fallback message ---
static std::string render_with_template(const kainjow::mustache::data& view) {
    std::string template_content = readFile("templates/figure_template.html");

    if (template_content.empty()) {
        return "<h3>template not found</h3>";
    }

    try {
        kainjow::mustache::mustache tpl(template_content);
        return tpl.render(view);
    } catch (const std::exception&) {
        return "<h3>template error: invalid mustache</h3>";
    }
}

// --- Constructor ---
Figure::Figure(const std::string& title)
    : _title(title) {
    resize(1, 1);
}

// --- Title ---
Figure& Figure::setTitle(const std::string& t) {
    _title = t;
    _dirty = true;
    return *this;
}

const std::string& Figure::title() const {
    return _title;
}

// --- Layout ---
Figure& Figure::setSubplot(uint32_t r, uint32_t c) {
    return resize(r, c);
}

Figure& Figure::setLayout(FigureLayout l) {
    _layout = std::move(l);
    remake();
    _dirty = true;
    return *this;
}

const FigureLayout& Figure::layout() const {
    return _layout;
}

uint32_t Figure::rows() const {
    return _layout.rows;
}

uint32_t Figure::cols() const {
    return _layout.cols;
}

uint32_t Figure::subplotCount() const {
    return static_cast<uint32_t>(_axes.size());
}

// --- Subplot access ---
Axis& Figure::subplot(uint32_t i) {
    if (i >= _axes.size()) {
        throw std::out_of_range("subplot index out of range");
    }
    return *_axes[i];
}

Axis& Figure::subplot(uint32_t r, uint32_t c) {
    return *_axes.at(idx(r, c));
}

// --- Rendering ---
std::string Figure::render() const {
    if (!_dirty && !_rendered.empty()) {
        return _rendered;
    }

    mustache::data view;
    view["title"] = _title;
    view["rows"] = static_cast<long long>(_layout.rows);
    view["cols"] = static_cast<long long>(_layout.cols);
    view["gap_px"] = _layout.gap_px;
    if (!_layout.css_class.empty()) {
        view["css_class"] = _layout.css_class;
    }

    mustache::data axes_list(mustache::data::type::list);
    for (const auto& ax_ptr : _axes) {
        mustache::data axis_data;

        if (ax_ptr->hasTitle()) {
            axis_data["has_title"] = true;
            axis_data["title"] = ax_ptr->title();
        } else {
            axis_data["has_title"] = false;
        }

        axis_data["inner_html"] = ax_ptr->render();
        axes_list.push_back(axis_data);
    }

    view.set("axes", axes_list);

    _rendered = render_with_template(view);
    _dirty = false;

    return _rendered;
}

// --- Private helpers ---
Figure& Figure::resize(uint32_t r, uint32_t c) {
    if (r == 0 || c == 0) {
        throw std::invalid_argument("rows and cols must be at least 1");
    }
    _layout.rows = r;
    _layout.cols = c;
    remake();
    _dirty = true;
    return *this;
}

void Figure::remake() {
    _axes.clear();
    _axes.reserve(_layout.rows * _layout.cols);
    for (uint32_t i = 0; i < _layout.rows * _layout.cols; ++i) {
        _axes.push_back(std::make_shared<Axis>());
    }
    _dirty = true;
}

uint32_t Figure::idx(uint32_t r, uint32_t c) const {
    if (r >= _layout.rows || c >= _layout.cols) {
        throw std::out_of_range("subplot index (row, col) out of range");
    }
    return r * _layout.cols + c;
}

} // namespace gsp