// GraphSvg.cpp
#include "libgsp/GraphSvg.h"

#include <algorithm>
#include <limits>
#include <utility>
#include <fstream>

#include <mustache.hpp>
#include <fmt/ranges.h>
#include <toml++/toml.h>

#include "libgsp/utils/Logging.h"          // gsp::logging::getLogger()
#include "libgsp/io/File.h"
#include "templates/graph_mustache_svg.h"  // templates::graph_mustache_svg

namespace gsp {

// -----------------------------------------------------------------------------
// GraphSvg::Impl — internal state + helpers only
// -----------------------------------------------------------------------------
struct GraphSvg::Impl {
    // snapshot of graph data
    std::vector<gsp::Node> nodes;
    std::vector<gsp::Edge> edges;

    // optional per-node signal
    std::vector<std::optional<double>> signal;

    // logging
    gsp::logging::Logger logger;

    // configuration registry (all defaults are registered here)
    toml::table config;

    // scalar mirrors (kept in sync with `config`)
    double node_space_scale{100.0};
    double signal_scale{100.0};
    std::string title{"Network"};
    std::string libgsp_version{"0.0.0"};
    std::string style_override;

    // bounds (lazy recompute)
    mutable double min_x{0}, max_x{0}, min_y{0}, max_y{0};

    // svg cache
    mutable std::string svg_cache;
    mutable bool dirty_svg{true};

    Impl(const gsp::BaseGraph& graph, std::optional<gsp::Signal<double>> signal_opt)
        : logger(gsp::logging::getLogger("GraphSvg")) {

        // Snapshot nodes & edges
        nodes = graph.nodes();
        edges = graph.edges();

        logger->trace("Creating GraphSvg with {} nodes and {} edges", nodes.size(), edges.size());
        if (signal_opt)
            logger->trace("Signal provided with size {}", signal_opt->size());

        // Initialize signal
        const auto n = nodes.size();
        signal.assign(n, std::optional<double>{});
        if (signal_opt) {
            if (signal_opt->size() == n) {
                signal = signal_opt->vector();
            } else {
                std::string msg = fmt::format("Signal size mismatch: got {}, expected {}", signal_opt->size(), n);
                logger->error(msg);
                throw std::invalid_argument(msg);
            }
        }
        logger->trace("Signal initialized with size {}", signal.size());
        if (signal_opt) logger->trace("Signal values: {}", signal_opt->str());

        // Register defaults in config registry
        config = toml::table{
            {"svg", toml::table{
                {"background", "gray"},
                {"css", ""}
            }},
            {"node", toml::table{
                {"fill",  "#00BCE3"},
                {"stroke","black"},
                {"radius", 8.0},
                {"opacity","0.8"}
            }},
            {"edge", toml::table{
                {"stroke","black"},
                {"width", 1.5}
            }},
            {"label", toml::table{
                {"fill","#000000"},
                {"font_px",12.0}
            }},
            {"signal", toml::table{
                {"color","red"},
                {"width",2.0},
                {"tip_radius",1.0},
                {"label", toml::table{
                    {"fill","red"},
                    {"font_px",6.0}
                }}
            }},
            {"scale", toml::table{
                {"node_space", node_space_scale},
                {"signal",     signal_scale}
            }}
        };
    }

    // Deep merge src into dst (tables only). Scalars/arrays overwrite.
    static void deep_merge(toml::table& dst, const toml::table& src) {
        for (auto&& [k, v] : src) {
            if (auto vtab = v.as_table()) {
                // ensure dst[k] exists and is a table
                auto it = dst.find(k);                // k is toml::key
                if (it == dst.end() || !it->second.is_table()) {
                    dst.insert_or_assign(k, toml::table{}); // create/overwrite as table
                    it = dst.find(k);
                }
                auto* dst_tab = it->second.as_table();      // guaranteed non-null
                deep_merge(*dst_tab, *vtab);
            } else {
                // overwrite scalars/arrays directly
                dst.insert_or_assign(k, v); // copies/converts node value as needed
            }
        }
    }


    void compute_bounds() const {
        double mnx =  std::numeric_limits<double>::infinity();
        double mxx = -std::numeric_limits<double>::infinity();
        double mny =  std::numeric_limits<double>::infinity();
        double mxy = -std::numeric_limits<double>::infinity();

        const double ns = node_space_scale;
        const double ss = signal_scale;

        for (size_t i = 0; i < nodes.size(); ++i) {
            const auto& c = nodes[i].coord;
            const double X  = ns * c.x;
            const double Y  = ns * c.y;
            const double sg = signal[i].value_or(0.0);
            const double Xs = X;
            const double Ys = Y + ss * sg;

            const double x_min = std::min(X, Xs) - 20.0;
            const double x_max = std::max(X, Xs) + 20.0;
            const double y_min = std::min(Y, Ys) - 20.0;
            const double y_max = std::max(Y, Ys) + 20.0;

            if (x_min < mnx) mnx = x_min;
            if (x_max > mxx) mxx = x_max;
            if (y_min < mny) mny = y_min;
            if (y_max > mxy) mxy = y_max;
        }

        if (nodes.empty()) { mnx = mny = 0.0; mxx = mxy = 1.0; }

        min_x = mnx; max_x = mxx; min_y = mny; max_y = mxy;
    }

    std::string build_metadata() const {
        fmt::memory_buffer buf;
        fmt::format_to(std::back_inserter(buf),
                       R"(<generator name="libgsp" author="Mohammad Raziei" site="https://mohammadraziei.github.io/libgsp" />)");
        fmt::format_to(std::back_inserter(buf),
                       R"(<libgsp version="{}" language="cpp" os="linux"/>)", libgsp_version);
        fmt::format_to(std::back_inserter(buf),
                       R"(<content format="svg" tags="plot graph signal bar"/>)");
        fmt::format_to(std::back_inserter(buf),
                       R"(<plotinfo signalscale="{}" nodespacescale="{}"/>)", signal_scale, node_space_scale);
        return {buf.data(), buf.size()};
    }

    std::string build_style() const {
        if (!style_override.empty())
            return style_override;

        // All keys are expected to exist in the registry (defaults or loaded).
        const auto& svg_tbl    = *config["svg"].as_table();
        const auto& node_tbl   = *config["node"].as_table();
        const auto& edge_tbl   = *config["edge"].as_table();
        const auto& label_tbl  = *config["label"].as_table();
        const auto& signal_tbl = *config["signal"].as_table();

        const std::string svg_bg  = svg_tbl["background"].value<std::string>().value();
        const std::string svg_css = svg_tbl["css"].value<std::string>().value();

        const std::string node_fill    = node_tbl["fill"].value<std::string>().value();
        const std::string node_stroke  = node_tbl["stroke"].value<std::string>().value();
        const std::string node_opacity = node_tbl["opacity"].value<std::string>().value();
        const double      node_radius  = node_tbl["radius"].value<double>().value(); // used inline in body

        const std::string edge_stroke  = edge_tbl["stroke"].value<std::string>().value();
        const double      edge_width   = edge_tbl["width"].value<double>().value();

        const std::string label_fill   = label_tbl["fill"].value<std::string>().value();
        const double      label_font   = label_tbl["font_px"].value<double>().value();

        const std::string sig_color    = signal_tbl["color"].value<std::string>().value();
        const double      sig_width    = signal_tbl["width"].value<double>().value();
        const double      sig_tip_r    = signal_tbl["tip_radius"].value<double>().value();
        const auto&       sig_lbl_tbl  = *signal_tbl["label"].as_table();
        const std::string sig_lbl_fill = sig_lbl_tbl["fill"].value<std::string>().value();
        const double      sig_lbl_font = sig_lbl_tbl["font_px"].value<double>().value();

        (void)node_radius;

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
            label_fill, label_font);
        fmt::format_to(std::back_inserter(css),
            "  .edge{{ stroke:{}; stroke-width:{} }}\n",
            edge_stroke, edge_width);
        fmt::format_to(std::back_inserter(css),
            "  .signal{{ stroke:{}; fill:{}; stroke-width:{} }}\n",
            sig_color, sig_color, sig_width);
        fmt::format_to(std::back_inserter(css),
            "  circle.signal{{ r:{} }}\n",
            sig_tip_r);
        fmt::format_to(std::back_inserter(css),
            "  .signal-text{{ fill:{} }}\n",
            sig_lbl_fill);

        return {css.data(), css.size()};
    }

    std::string build_body() const {
        // All keys must exist in registry
        const double node_radius  = config["node"]["radius"].value<double>().value();
        const double sig_lbl_font = config["signal"]["label"]["font_px"].value<double>().value();

        fmt::memory_buffer body;

        // Edges
        for (const auto& e : edges) {
            const auto& s = nodes[e.source].coord;
            const auto& t = nodes[e.target].coord;

            const double x1 = node_space_scale * s.x - min_x;
            const double y1 = -node_space_scale * s.y + max_y;
            const double x2 = node_space_scale * t.x - min_x;
            const double y2 = -node_space_scale * t.y + max_y;

            fmt::format_to(std::back_inserter(body),
                R"(  <line class="edge" x1="{}" y1="{}" x2="{}" y2="{}" v1="{}" v2="{}" weight="{}"/>)"
                "\n", x1, y1, x2, y2, e.source, e.target, e.weight);
        }

        // Nodes + signals
        for (size_t i = 0; i < nodes.size(); ++i) {
            const auto& node = nodes[i];
            const double X = node_space_scale * node.coord.x - min_x;
            const double Y = -node_space_scale * node.coord.y + max_y;
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

            if (signal[i].has_value()) {
                const double s = *signal[i];
                const double dy = -s * signal_scale; // SVG Y goes down
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

    std::string render_with_template(double width,
                                     double height,
                                     const std::string& metadata,
                                     const std::string& style,
                                     const std::string& body) const {
        logger->trace("rendering with config (toml)");
        kainjow::mustache::data ctx;
        ctx.set("width",  fmt::format("{}", width));
        ctx.set("height", fmt::format("{}", height));
        ctx.set("title",  title);
        ctx.set("metadata", metadata); // template should use {{{metadata}}}
        ctx.set("style",   style);     // {{{style}}}
        ctx.set("body",    body);      // {{{body}}}

        const std::string& tpl_str = templates::graph_mustache_svg;

        kainjow::mustache::mustache tpl(tpl_str);
        if (!tpl.is_valid()) {
            logger->error("Mustache template error: {}", tpl.error_message());
            return fmt::format("<!-- template error: {} -->", tpl.error_message());
        }
        return tpl.render(ctx);
    }
};

// -----------------------------------------------------------------------------
// Public API (implemented directly; Impl holds only data/helpers)
// -----------------------------------------------------------------------------
GraphSvg::GraphSvg(const gsp::BaseGraph& graph, std::optional<Signal<double>> signal)
    : pimpl(std::make_unique<Impl>(graph, std::move(signal))) {}

GraphSvg::~GraphSvg() = default;
GraphSvg::GraphSvg(GraphSvg&&) noexcept = default;
GraphSvg& GraphSvg::operator=(GraphSvg&&) noexcept = default;

// ------------------ Config I/O: load / loads / dump / dumps -------------------

GraphSvg& GraphSvg::loadConfig(const std::string& file_path) {
    // Parse TOML file then deep-merge into registry
    auto parsed = toml::parse_file(file_path);
    auto tbl = parsed.as_table();
    if (!tbl) {
        pimpl->logger->error("load_config: file '{}' did not parse to a table", file_path);
        throw std::runtime_error("TOML root is not a table");
    }
    Impl::deep_merge(pimpl->config, *tbl);

    // Sync mirrors if present in the loaded config
    if (auto v = pimpl->config["scale"]["node_space"].value<double>())
        pimpl->node_space_scale = *v;
    if (auto v = pimpl->config["scale"]["signal"].value<double>())
        pimpl->signal_scale = *v;

    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::loadsConfig(const std::string& toml_text) {
    auto parsed = toml::parse(toml_text);
    auto tbl = parsed.as_table();
    if (!tbl) {
        pimpl->logger->error("loads_config: input did not parse to a table");
        throw std::runtime_error("TOML root is not a table");
    }
    Impl::deep_merge(pimpl->config, *tbl);

    if (auto v = pimpl->config["scale"]["node_space"].value<double>())
        pimpl->node_space_scale = *v;
    if (auto v = pimpl->config["scale"]["signal"].value<double>())
        pimpl->signal_scale = *v;

    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::dumpConfig(const std::string& file_path) const {
    std::ofstream ofs(file_path);
    if (!ofs) {
        pimpl->logger->error("dumpConfig: cannot open '{}' for writing", file_path);
        throw std::runtime_error("dumpConfig: open failed");
    }

    ofs << pimpl->config; // << works for toml::table
    pimpl->logger->info("Saved TOML config to {}", file_path);
    return const_cast<GraphSvg&>(*this);
}

std::string GraphSvg::dumpsConfig() const {
    return fmt::format("{}", fmt::streamed(pimpl->config));
}


// ------------------------------ Other setters --------------------------------

GraphSvg& GraphSvg::setTitle(const std::string& title) {
    pimpl->title = title;
    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::setLibGspVersion(const std::string& version) {
    pimpl->libgsp_version = version;
    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::setNodeSpaceScale(double v) {
    pimpl->node_space_scale = v;
    // assumes "scale" table and "node_space" key already exist
    pimpl->config.get("scale")->as_table()->insert_or_assign("node_space", v);
    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::setSignalScale(double v) {
    pimpl->signal_scale = v;
    // assumes "scale" table and "signal" key already exist
    pimpl->config.get("scale")->as_table()->insert_or_assign("signal", v);
    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::setStyleOverride(const std::string& css) {
    pimpl->style_override = css;
    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::setSignal(const Eigen::VectorXd& s) {
    const auto n = pimpl->nodes.size();
    if (s.size() != static_cast<Eigen::Index>(n)) {
        pimpl->logger->warn("setSignal size mismatch: got {}, expected {}", s.size(), n);
        return *this;
    }
    for (size_t i = 0; i < n; ++i)
        pimpl->signal[i] = s(static_cast<Eigen::Index>(i));
    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::addSignal(uint32_t idx, double value) {
    if (idx >= pimpl->signal.size()) {
        pimpl->logger->warn("addSignal: index {} out of range (n={})", idx, pimpl->signal.size());
        return *this;
    }
    pimpl->signal[idx] = value;
    pimpl->dirty_svg = true;
    return *this;
}

// --------------------------------- Render ------------------------------------

const std::string& GraphSvg::render() const {
    if (!pimpl->dirty_svg) return pimpl->svg_cache;

    pimpl->compute_bounds();

    const double width  = std::max(1.0, pimpl->max_x - pimpl->min_x);
    const double height = std::max(1.0, pimpl->max_y - pimpl->min_y);

    const std::string metadata = pimpl->build_metadata();
    const std::string style    = pimpl->build_style();
    const std::string body     = pimpl->build_body();

    pimpl->svg_cache = pimpl->render_with_template(width, height, metadata, style, body);
    pimpl->dirty_svg = false;
    return pimpl->svg_cache;
}

const std::string& GraphSvg::svg() const {
    return render();
}

void GraphSvg::save(const std::string& filepath) const {
    const auto svg_s = render();
    gsp::io::writeFile(filepath, svg_s);
    pimpl->logger->info("Saved SVG to {}", filepath);
}

// Convenience for quick theming edits
GraphSvg& GraphSvg::setSvgCss(const std::string& css) {
    auto* svg_tbl = pimpl->config.get("svg")->as_table(); // must exist
    svg_tbl->insert_or_assign("css", css);
    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::setSvgBackground(const std::string& bg) {
    auto* svg_tbl = pimpl->config.get("svg")->as_table(); // must exist
    svg_tbl->insert_or_assign("background", bg);
    pimpl->dirty_svg = true;
    return *this;
}

} // namespace gsp
