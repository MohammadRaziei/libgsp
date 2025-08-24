//
// Created by Mohammad on 8/23/2025.
//

#include "libgsp/plotting/figure.h"
#include "libgsp/io/file.h"
#include "libgsp/plotting/axis.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <mustache.hpp>


// -------------------------- bulk set --------------------------
Figure& Figure::setSubplots(const std::vector<Axis*>& axes) {
    const auto required = static_cast<size_t>(_layout.rows) * _layout.cols;
    if (axes.size() != required) {
        throw std::invalid_argument("setSubplots: size mismatch (expected rows*cols)");
    }
    _axes = axes;
    _dirty = true;
    return *this;
}

// -------------------------- set single cell -------------------
Figure& Figure::setSubplot(uint32_t r, uint32_t c, Axis* ax) {
    _axes.at(idx(r, c)) = ax;
    _dirty = true;
    return *this;
}

// -------------------------- render (cached, mustache) ---------
std::string Figure::render(const std::string& templatePath) const {
    if (!_dirty && !_rendered.empty())
        return _rendered;

    using kainjow::mustache::mustache;
    using kainjow::mustache::data;
    using kainjow::mustache::list;

    // Load template file
    const std::string tpl_text = readFile(templatePath);
    mustache tmpl(tpl_text);

    // Build root context
    data root;
    root["title"]     = _title;
    root["gap_px"]    = std::to_string(_layout.gap_px);
    root["cols"]      = std::to_string(_layout.cols);
    root["css_class"] = _layout.css_class;

    list axes_list;
    axes_list.reserve(_axes.size());

    for (size_t i = 0; i < _axes.size(); ++i) {
        data axd;

        if (const Axis* ax = _axes[i]) {
            const auto& c = ax->common();
            axd["has_title"]  = !c.title.empty() ? "true" : "";
            axd["title"]      = c.title;
            axd["inner_html"] = ax->render(); // raw fragment
        } else {
            axd["has_title"]  = "";
            axd["title"]      = "";
            axd["inner_html"] = "<em>Empty subplot (" + std::to_string(i) + ")</em>";
        }

        axes_list.push_back(axd);
    }

    root["axes"] = axes_list;

    // Render & cache
    _rendered = tmpl.render(root);
    _dirty = false;
    return _rendered;
}
