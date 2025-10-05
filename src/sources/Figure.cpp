//
// Created by Mohammad on 8/23/2025.
//

#include "libgsp/plotting/Figure.h"
#include "libgsp/io/File.h"
#include "libgsp/plotting/Axis.h"
#include "libgsp/utils/GspInfo.h"

#include <filesystem>
#include <stdexcept>
#include <set>


#include <mustache.hpp>

#include "templates/figure_mustache_html.h"
#include "Templates.h"

namespace fs = std::filesystem;
using namespace kainjow;

namespace gsp {



// --- Constructor ---
Figure::Figure(const std::string& title)
    : _title(title), _logger(gsp::logging::getLogger("Figure")) {
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

Axis& Figure::subplot(const std::set<uint32_t>& indices) {
    if (indices.empty()) {
        throw std::invalid_argument("Subplot indices set is empty.");
    }

    const uint32_t total = subplotCount();
    const uint32_t grid_cols = cols();

    // Validate all indices are in range
    for (uint32_t i : indices) {
        if (i >= total) {
            throw std::out_of_range("Subplot index " + std::to_string(i) + " out of range [0, " + std::to_string(total - 1) + "]");
        }
    }

    // indices is already sorted and unique (property of std::set)

    uint32_t min_r = UINT32_MAX, max_r = 0;
    uint32_t min_c = UINT32_MAX, max_c = 0;
    std::set<std::pair<uint32_t, uint32_t>> cell_set;

    for (uint32_t idx : indices) {
        uint32_t r = idx / grid_cols;
        uint32_t c = idx % grid_cols;
        min_r = std::min(min_r, r);
        max_r = std::max(max_r, r);
        min_c = std::min(min_c, c);
        max_c = std::max(max_c, c);
        cell_set.insert({r, c});
    }

    uint32_t expected = (max_r - min_r + 1) * (max_c - min_c + 1);
    if (indices.size() != expected) {
        throw std::invalid_argument("Indices do not form a contiguous rectangular block.");
    }

    // Final verification
    for (uint32_t r = min_r; r <= max_r; ++r) {
        for (uint32_t c = min_c; c <= max_c; ++c) {
            if (cell_set.count({r, c}) == 0) {
                throw std::invalid_argument("Indices do not form a contiguous rectangular block (missing cell).");
            }
        }
    }

    auto shared_axis = _axes[*indices.begin()];

    // Point all indices in the block to this shared axis
    for (uint32_t idx : indices) {
        _axes[idx] = shared_axis;
    }

    return *shared_axis;
}
// --- Rendering ---
std::string Figure::render() const {
    if (!_dirty && !_rendered.empty()) {
        return _rendered;
    }

    mustache::data view;
    view["title"] = _title;
    view["initial_comments"] = templates::getInitialComments();
    view["rows"] = static_cast<long long>(_layout.rows);
    view["cols"] = static_cast<long long>(_layout.cols);
    view["gap_px"] = _layout.gap_px;
    if (!_layout.css_class.empty()) {
        view["css_class"] = _layout.css_class;
    }

    // Add GspInfo meta tags
    const auto& info = gsp::info::GspInfo::instance();
    view["gsp_name"] = info.name();
    view["gsp_version"] = info.version();
    view["gsp_author_name"] = info.authorName();
    view["gsp_author_email"] = info.authorEmail();
    view["gsp_site_url"] = info.siteUrl();
    view["gsp_language"] = info.language();
    view["gsp_os"] = info.os();

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
    try {
        mustache::mustache tpl(templates::figure_mustache_html);
        _rendered = tpl.render(view);
    } catch (const std::exception&) {
        return "<h3>template error: invalid mustache</h3>";
    }
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

uint32_t Figure::numSubplots() const {
    return std::set(_axes.begin(), _axes.end()).size();
}

} // namespace gsp