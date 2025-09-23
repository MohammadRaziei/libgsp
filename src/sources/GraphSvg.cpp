// PlotSvg.cpp
#include "libgsp/GraphSvg.h"

#include <algorithm>
#include <limits>
#include <utility>

#include <mustache.hpp>
#include <fmt/ranges.h>

#include "libgsp/utils/Logging.h"          // gsp::logging::getLogger()
#include "libgsp/io/File.h"

#include "templates/graph_mustache_svg.h"  // templates::graph_mustache_svg

#include "libgsp/utils/Logging.h"

namespace gsp {

// -----------------------------------------------------------------------------
// PlotSvg::Impl declaration (PIMPL) — prototypes first, definitions later
// -----------------------------------------------------------------------------
struct GraphSvg::Impl {
    // --- ctor/dtor ---
    Impl(const gsp::BaseGraph& graph, std::optional<gsp::Signal<double>> signal_opt);
    ~Impl() = default;

    // --- configuration API used by the outer class ---
    void set_config(const std::unordered_map<std::string, std::string>& cfg);
    void set_option(std::string key, std::string value);
    void set_title(const std::string& title);
    void set_version(const std::string& version);
    void set_node_space_scale(double v);
    void set_signal_scale(double v);
    void set_style_override(const std::string& css);
    void set_signal(const Eigen::VectorXd& s);
    void add_signal(uint32_t idx, double value);

    // --- public render entry for outer class ---
    const std::string& make_svg();  // uses cache


    // --- internal helpers (split for readability) ---
    void _compute_bounds() const;
    std::string _build_metadata() const;
    std::string _build_style() const;
    std::string _build_body() const;
    std::string _render_with_template(double width,
                                      double height,
                                      const std::string& metadata,
                                      const std::string& style,
                                      const std::string& body) const;

    // --- utility ---
    double _cfg_double(const char* key) const;
    double _cfg_double(const char* key, double defval) const;
    std::string _cfg_str(const char* key) const;
    std::string _cfg_str(const char* key, const char* defval) const;

    // snapshot of graph data
    std::vector<gsp::Node> _nodes;
    std::vector<gsp::Edge> _edges;

    // signals (optional per node)
    std::vector<std::optional<double>> _signal;

    // logging
    gsp::logging::Logger _logger;

    // user-config map (stringly-typed for maximal flexibility)
    std::unordered_map<std::string, std::string> _config;

    // scalar settings (mirrored in _config when appropriate)
    double _node_space_scale{100.0};
    double _signal_scale{100.0};
    std::string _title{"Network"};
    std::string _libgsp_version{"0.0.0"};
    std::string _style_override;

    // bounds (mutable for lazy recompute)
    mutable double _min_x{0}, _max_x{0}, _min_y{0}, _max_y{0};

    // svg cache
    mutable std::string _svg_cache;
    mutable bool _dirty_svg{true};
};

// -----------------------------------------------------------------------------
// PlotSvg public API
// -----------------------------------------------------------------------------
GraphSvg::GraphSvg(const gsp::BaseGraph& graph,
                 std::optional<Signal<double>> signal)
    : pimpl(std::make_unique<Impl>(graph, std::move(signal))) {}

GraphSvg::~GraphSvg() = default;
GraphSvg::GraphSvg(GraphSvg&&) noexcept = default;
GraphSvg& GraphSvg::operator=(GraphSvg&&) noexcept = default;

GraphSvg& GraphSvg::setConfig(const std::unordered_map<std::string, std::string>& cfg) {
    pimpl->set_config(cfg);
    return *this;
}
GraphSvg& GraphSvg::setOption(const std::string& key, const std::string& value) {
    pimpl->set_option(key, value);
    return *this;
}
GraphSvg& GraphSvg::setTitle(const std::string& title) {
    pimpl->set_title(title);
    return *this;
}
GraphSvg& GraphSvg::setLibGspVersion(const std::string& version) {
    pimpl->set_version(version);
    return *this;
}
GraphSvg& GraphSvg::setNodeSpaceScale(double v) {
    pimpl->set_node_space_scale(v);
    return *this;
}
GraphSvg& GraphSvg::setSignalScale(double v) {
    pimpl->set_signal_scale(v);
    return *this;
}
GraphSvg& GraphSvg::setStyleOverride(const std::string& css) {
    pimpl->set_style_override(css);
    return *this;
}
GraphSvg& GraphSvg::setSignal(const Eigen::VectorXd& s) {
    pimpl->set_signal(s);
    return *this;
}
GraphSvg& GraphSvg::addSignal(uint32_t idx, double value) {
    pimpl->add_signal(idx, value);
    return *this;
}
const std::string& GraphSvg::render() const {
    return pimpl->make_svg();
}
const std::string& GraphSvg::svg() const {
    return render();
}
void GraphSvg::save(const std::string& filepath) const {
    const auto svg = pimpl->make_svg();
    gsp::io::writeFile(filepath, svg);
    pimpl->_logger->info("Saved SVG to {}", filepath);
}

GraphSvg& GraphSvg::setSvgCss(const std::string& css) {
    pimpl->_config.emplace("svg.css", css);
    pimpl->_dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::setSvgBackground(const std::string& css) {
    pimpl->_config.emplace("svg.background", css);
    pimpl->_dirty_svg = true;
    return *this;
}
// -----------------------------------------------------------------------------
// PlotSvg::Impl definitions
// -----------------------------------------------------------------------------
GraphSvg::Impl::Impl(const gsp::BaseGraph& graph,
                    std::optional<gsp::Signal<double>> signal_opt)
    : _logger(gsp::logging::getLogger("GraphSvg")) {

    // snapshot nodes/edges once (no further graph calls)
    _nodes = graph.nodes();
    _edges = graph.edges();

    _logger->trace("Creating GraphSvg with {} nodes and {} edges",
        _nodes.size(), _edges.size());

    if (signal_opt) {
        _logger->trace("Signal provided with size {}", signal_opt->size());
    }

    // init signal
    const auto n = _nodes.size();
    _signal.assign(n, std::optional<double>{});
    if (signal_opt) {
        if (signal_opt->size() == n) {
            _signal = signal_opt->vector();
        } else {
            std::string msg = fmt::format("Signal size mismatch: got {}, expected {}",
                                          signal_opt->size(), n);
            _logger->error(msg);
            throw std::invalid_argument(msg);
        }
    }

    _logger->trace("Signal initialized with size {}", _signal.size());
    if (signal_opt) {
        _logger->trace("Signal values: {}", signal_opt->str());
    }


    // defaults
    _config.emplace("svg.background", "gray");
    _config.emplace("svg.css", "");
    _config.emplace("node.fill",  "#00BCE3");
    _config.emplace("node.stroke","black");
    _config.emplace("node.radius","8");
    _config.emplace("node.opacity","0.8");

    _config.emplace("edge.stroke","black");
    _config.emplace("edge.width","1.5");

    _config.emplace("label.fill","#000000");
    _config.emplace("label.font_px","12");

    _config.emplace("signal.color","red");
    _config.emplace("signal.width","2");
    _config.emplace("signal.tip_radius","1");
    _config.emplace("signal.label.fill","red");
    _config.emplace("signal.label.font_px","6");

    _config.emplace("scale.node_space", fmt::format("{}", _node_space_scale));
    _config.emplace("scale.signal",     fmt::format("{}", _signal_scale));

    _dirty_svg = true;
}

void GraphSvg::Impl::set_config(const std::unordered_map<std::string, std::string>& cfg) {
    _config = cfg;
    if (auto it = _config.find("scale.node_space"); it != _config.end()) {
        try { _node_space_scale = std::stod(it->second); } catch (...) {}
    }
    if (auto it = _config.find("scale.signal"); it != _config.end()) {
        try { _signal_scale = std::stod(it->second); } catch (...) {}
    }
    _dirty_svg = true;
}
void GraphSvg::Impl::set_option(std::string key, std::string value) {
    if (key == "scale.node_space") { try { _node_space_scale = std::stod(value); } catch (...) {} }
    if (key == "scale.signal")     { try { _signal_scale     = std::stod(value); } catch (...) {} }
    _config[std::move(key)] = std::move(value);
    _dirty_svg = true;
}
void GraphSvg::Impl::set_title(const std::string& title) {
    _title = title; _dirty_svg = true;
}
void GraphSvg::Impl::set_version(const std::string& version) {
    _libgsp_version = version; _dirty_svg = true;
}
void GraphSvg::Impl::set_node_space_scale(double v) {
    _node_space_scale = v;
    _config["scale.node_space"] = fmt::format("{}", v);
    _dirty_svg = true;
}
void GraphSvg::Impl::set_signal_scale(double v) {
    _signal_scale = v;
    _config["scale.signal"] = fmt::format("{}", v);
    _dirty_svg = true;
}

void GraphSvg::Impl::set_style_override(const std::string& css) {
    _style_override = css; _dirty_svg = true;
}
void GraphSvg::Impl::set_signal(const Eigen::VectorXd& s) {
    const auto n = _nodes.size();
    if (s.size() != static_cast<Eigen::Index>(n)) {
        _logger->warn("setSignal size mismatch: got {}, expected {}", s.size(), n);
        return;
    }
    for (size_t i = 0; i < n; ++i) _signal[i] = s(static_cast<Eigen::Index>(i));
    _dirty_svg = true;
}
void GraphSvg::Impl::add_signal(uint32_t idx, double value) {
    if (idx >= _signal.size()) {
        _logger->warn("addSignal: index {} out of range (n={})", idx, _signal.size());
        return;
    }
    _signal[idx] = value;
    _dirty_svg = true;
}

const std::string& GraphSvg::Impl::make_svg() {
    if (!_dirty_svg) return _svg_cache;

    _compute_bounds();

    const double width  = std::max(1.0, _max_x - _min_x);
    const double height = std::max(1.0, _max_y - _min_y);

    const std::string metadata = _build_metadata();
    const std::string style    = _build_style();
    const std::string body     = _build_body();

    _svg_cache = _render_with_template(width, height, metadata, style, body);
    _dirty_svg = false;
    return _svg_cache;
}

// --- internal helpers ---------------------------------------------------------
double GraphSvg::Impl::_cfg_double(const char* key, double defval) const {
    try { return std::stod(_cfg_str(key)); } catch (...) {}
    return defval;
}
double GraphSvg::Impl::_cfg_double(const char* key) const {
    try { return std::stod(_cfg_str(key)); } catch (...) {
        throw std::runtime_error(fmt::format("Key {} not found", key));
    }
}
std::string GraphSvg::Impl::_cfg_str(const char* key, const char* defval) const {
    if (auto it = _config.find(key); it != _config.end()) return it->second;
    return defval;
}

std::string GraphSvg::Impl::_cfg_str(const char* key) const {
    if (auto it = _config.find(key); it != _config.end()) return it->second;
    throw std::runtime_error(fmt::format("Key {} not found", key));
}

void GraphSvg::Impl::_compute_bounds() const {
    double mnx =  std::numeric_limits<double>::infinity();
    double mxx = -std::numeric_limits<double>::infinity();
    double mny =  std::numeric_limits<double>::infinity();
    double mxy = -std::numeric_limits<double>::infinity();

    for (size_t i = 0; i < _nodes.size(); ++i) {
        const auto& c = _nodes[i].coord;
        const double X  = _node_space_scale * c.x;
        const double Y  = _node_space_scale * c.y;
        const double sg = _signal[i].value_or(0.0);
        const double Xs = X;
        const double Ys = Y + _signal_scale * sg;

        const double x_min = std::min(X, Xs) - 20.0;
        const double x_max = std::max(X, Xs) + 20.0;
        const double y_min = std::min(Y, Ys) - 20.0;
        const double y_max = std::max(Y, Ys) + 20.0;

        if (x_min < mnx) mnx = x_min;
        if (x_max > mxx) mxx = x_max;
        if (y_min < mny) mny = y_min;
        if (y_max > mxy) mxy = y_max;
    }

    if (_nodes.empty()) { mnx = mny = 0.0; mxx = mxy = 1.0; }

    _min_x = mnx; _max_x = mxx; _min_y = mny; _max_y = mxy;
}

std::string GraphSvg::Impl::_build_metadata() const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf),
                   R"(<generator name="libgsp" author="Mohammad Raziei" site="https://mohammadraziei.github.io/libgsp" />)");
    fmt::format_to(std::back_inserter(buf),
                   R"(<libgsp version="{}" language="cpp" os="linux"/>)", _libgsp_version);
    fmt::format_to(std::back_inserter(buf),
                   R"(<content format="svg" tags="plot graph signal bar"/>)");
    fmt::format_to(std::back_inserter(buf),
                   R"(<plotinfo signalscale="{}" nodespacescale="{}"/>)", _signal_scale, _node_space_scale);
    return {buf.data(), buf.size()};
}

std::string GraphSvg::Impl::_build_style() const {
    if (!_style_override.empty()) return _style_override;

    const auto svg_bg        = _cfg_str("svg.background");
    const auto svg_css        = _cfg_str("svg.css");

    const auto node_fill      = _cfg_str("node.fill");
    const auto node_stroke    = _cfg_str("node.stroke");
    const auto node_opacity   = _cfg_str("node.opacity");
    const auto node_radius    = _cfg_double("node.radius");

    const auto edge_stroke    = _cfg_str("edge.stroke");
    const auto edge_width     = _cfg_double("edge.width");

    const auto label_fill     = _cfg_str("label.fill");
    const auto label_font_px  = _cfg_double("label.font_px");

    const auto signal_color   = _cfg_str("signal.color");
    const auto signal_width   = _cfg_double("signal.width");
    const auto signal_tip_r   = _cfg_double("signal.tip_radius");
    const auto sig_lbl_fill   = _cfg_str("signal.label.fill");
    const auto sig_lbl_font   = _cfg_double("signal.label.font_px");

    (void)node_radius; // applied inline in body

    fmt::memory_buffer css;
    fmt::format_to(std::back_inserter(css),
               "  svg{{ {1};{0} }}\n",
               svg_bg.empty() ? "" : fmt::format(" background:{};", svg_bg),
               svg_css);
    fmt::format_to(std::back_inserter(css),
                   "  .node{{ fill:{}; stroke:{}; stroke-width:.7; opacity:{} }}\n",
                   node_fill, node_stroke, node_opacity);
    fmt::format_to(std::back_inserter(css),
                   "  .node-label{{ fill:{}; font-size:{}; text-anchor:middle; dominant-baseline:middle }}\n",
                   label_fill, label_font_px);
    fmt::format_to(std::back_inserter(css),
                   "  .edge{{ stroke:{}; stroke-width:{} }}\n",
                   edge_stroke, edge_width);
    fmt::format_to(std::back_inserter(css),
                   "  .signal{{ stroke:{}; fill:{}; stroke-width:{} }}\n",
                   signal_color, signal_color, signal_width);
    fmt::format_to(std::back_inserter(css),
                   "  circle.signal{{ r:{} }}\n",
                   signal_tip_r);
    fmt::format_to(std::back_inserter(css),
                   "  .signal-text{{ fill:{} }}\n",
                   sig_lbl_fill);

    return {css.data(), css.size()};
}

std::string GraphSvg::Impl::_build_body() const {
    const double node_radius  = _cfg_double("node.radius");
    const double sig_lbl_font = _cfg_double("signal.label.font_px");

    fmt::memory_buffer body;

    // edges
    for (const auto& e : _edges) {
        const auto& s = _nodes[e.source].coord;
        const auto& t = _nodes[e.target].coord;

        const double x1 = _node_space_scale * s.x - _min_x;
        const double y1 = -_node_space_scale * s.y + _max_y;
        const double x2 = _node_space_scale * t.x - _min_x;
        const double y2 = -_node_space_scale * t.y + _max_y;

        fmt::format_to(std::back_inserter(body),
                       R"(  <line class="edge" x1="{}" y1="{}" x2="{}" y2="{}" v1="{}" v2="{}" weight="{}"/>)"
                       "\n", x1, y1, x2, y2, e.source, e.target, e.weight);
    }

    // nodes + signals
    for (size_t i = 0; i < _nodes.size(); ++i) {
        const auto& node = _nodes[i];
        const double X = _node_space_scale * node.coord.x - _min_x;
        const double Y = -_node_space_scale * node.coord.y + _max_y;
        const double Z = node.coord.z;
        const std::string name = node.name.empty()
                                 ? fmt::format("v{}", node.id)
                                 : node.name;

        fmt::format_to(std::back_inserter(body),
                       R"(  <circle class="node" cx="{}" cy="{}" r="{}" vx="{}" vy="{}" vz="{}" v="{}" name="{}"/>)"
                       "\n", X, Y, node_radius, node.coord.x, node.coord.y, Z, node.id, name);

        fmt::format_to(std::back_inserter(body),
                       R"(  <text class="node-label" x="{}" y="{}">{}</text>)"
                       "\n", X, Y, name);

        if (_signal[i].has_value()) {
            const double s = *_signal[i];
            const double dy = -s * _signal_scale; // SVG Y goes down
            const double x2 = X, y2 = Y + dy;

            fmt::format_to(std::back_inserter(body),
                           R"(  <line class="signal" x1="{}" y1="{}" x2="{}" y2="{}"/>)"
                           "\n", X, Y, x2, y2);

            fmt::format_to(std::back_inserter(body),
                           R"(  <circle class="signal" cx="{}" cy="{}" r="1"/>)"
                           "\n", x2, y2);

            const double ty = y2 + ((dy < 0.0) ? -sig_lbl_font : sig_lbl_font);
            fmt::format_to(std::back_inserter(body),
                           R"(  <text class="signal-text" text-anchor="middle" dominant-baseline="middle" font-size="{}px" x="{}" y="{}">{}</text>)"
                           "\n", sig_lbl_font, X, ty, s);
        }
    }

    return {body.data(), body.size()};
}

std::string GraphSvg::Impl::_render_with_template(double width,
                                                 double height,
                                                 const std::string& metadata,
                                                 const std::string& style,
                                                 const std::string& body) const {
    _logger->trace("rendering with config: {}", _config);
    kainjow::mustache::data ctx;
    ctx.set("width",  fmt::format("{}", width));
    ctx.set("height", fmt::format("{}", height));
    ctx.set("title",  _title);
    ctx.set("metadata", metadata); // template should use {{{metadata}}}
    ctx.set("style",   style);     // {{{style}}}
    ctx.set("body",    body);      // {{{body}}}

    const std::string& tpl_str = templates::graph_mustache_svg;

    kainjow::mustache::mustache tpl(tpl_str);
    if (!tpl.is_valid()) {
        _logger->error("Mustache template error: {}", tpl.error_message());
        return fmt::format("<!-- template error: {} -->", tpl.error_message());
    }
    return tpl.render(ctx);
}

} // namespace gsp
