// GraphSvg.cpp
#include "libgsp/GraphSvg.h"

#include <algorithm>
#include <limits>
#include <utility>
#include <fstream>
#include <memory>
#include <sstream>

#include <mustache.hpp>
#include <fmt/ranges.h>

#include <pugixml.hpp>

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

    // configuration registry (XML, shared)
    // root element: <config> ... </config>
    std::shared_ptr<pugi::xml_document> config;

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

        // logger->trace("Creating GraphSvg with {} nodes and {} edges", nodes.size(), edges.size());
        // if (signal_opt)
        //     logger->trace("Signal provided with size {}", signal_opt->size());

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
        // logger->trace("Signal initialized with size {}", signal.size());
        // if (signal_opt) logger->trace("Signal values: {}", signal_opt->str());

        // Build default XML config registry
        config = std::make_shared<pugi::xml_document>();
        auto cfg = config->append_child("config");

        auto svg = cfg.append_child("svg");
        svg.append_attribute("background") = "gray";
        svg.append_attribute("css") = "";

        auto node = cfg.append_child("node");
        node.append_attribute("fill")   = "#00BCE3";
        node.append_attribute("stroke") = "black";
        node.append_attribute("radius") = 8.0;
        node.append_attribute("opacity")= "0.8";

        auto edge = cfg.append_child("edge");
        edge.append_attribute("stroke") = "black";
        edge.append_attribute("width")  = 1.5;

        auto label = cfg.append_child("label");
        label.append_attribute("fill")    = "#000000";
        label.append_attribute("font_px") = 12.0;

        auto signal_n = cfg.append_child("signal");
        signal_n.append_attribute("color")      = "red";
        signal_n.append_attribute("width")      = 2.0;
        signal_n.append_attribute("tip_radius") = 1.0;
        auto siglbl = signal_n.append_child("label");
        siglbl.append_attribute("fill")    = "red";
        siglbl.append_attribute("font_px") = 6.0;

        auto scale = cfg.append_child("scale");
        scale.append_attribute("node_space") = node_space_scale;
        scale.append_attribute("signal")     = signal_scale;
    }

    // Merge src subtree into dst subtree by element name; attributes overwrite; missing children are created.
    static void xml_deep_merge(pugi::xml_node dst, pugi::xml_node src) {
        // overwrite/add attributes
        for (auto a : src.attributes()) {
            auto da = dst.attribute(a.name());
            if (da) da.set_value(a.value());
            else    dst.append_attribute(a.name()).set_value(a.value());
        }
        // recurse on child elements by tag-name
        for (auto c : src.children()) {
            if (c.type() != pugi::node_element) continue;
            auto dn = dst.child(c.name());
            if (!dn) dn = dst.append_child(c.name());
            xml_deep_merge(dn, c);
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

            double sg = 0.0;
            if (signal[i].has_value()) sg = *signal[i];

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

    // Embed full config inside <metadata><plotinfo>...</plotinfo></metadata>
    std::string build_metadata() const {
        pugi::xml_document tmp;
        auto md = tmp.append_child("metadata");

        // generator (optional)
        auto gen = md.append_child("generator");
        gen.append_attribute("name")   = "libgsp";
        gen.append_attribute("author") = "Mohammad Raziei";
        gen.append_attribute("site")   = "https://mohammadraziei.github.io/libgsp";

        // libgsp info (optional)
        auto lg = md.append_child("libgsp");
        lg.append_attribute("version")  = libgsp_version.c_str();
        lg.append_attribute("language") = "cpp";
        lg.append_attribute("os")       = "linux";

        // content (optional)
        auto content = md.append_child("content");
        content.append_attribute("format") = "svg";
        content.append_attribute("tags")   = "plot graph signal bar";

        // plotinfo + attributes + embedded <config>
        auto pi = md.append_child("plotinfo");
        pi.append_attribute("signalscale")    = signal_scale;
        pi.append_attribute("nodespacescale") = node_space_scale;

        // embed config subtree
        auto cfg_src = config->child("config");
        auto cfg_dst = pi.append_child("config");
        for (auto child : cfg_src.children()) {
            cfg_dst.append_copy(child);
        }

        std::ostringstream oss;
        // tmp.save(oss, "  ", pugi::format_no_declaration);
        md.print(oss, "  ");
        return oss.str();
    }

    std::string build_style() const {
        if (!style_override.empty())
            return style_override;

        auto cfg    = config->child("config");
        auto svg    = cfg.child("svg");
        auto node   = cfg.child("node");
        auto edge   = cfg.child("edge");
        auto label  = cfg.child("label");
        auto signal = cfg.child("signal");
        auto siglbl = signal.child("label");

        const char* svg_bg  = svg.attribute("background").value();
        const char* svg_css = svg.attribute("css").value();

        const char* node_fill    = node.attribute("fill").value();
        const char* node_stroke  = node.attribute("stroke").value();
        const char* node_opacity = node.attribute("opacity").value();
        const double node_radius = node.attribute("radius").as_double(); // used inline in body

        const char* edge_stroke  = edge.attribute("stroke").value();
        const double edge_width  = edge.attribute("width").as_double();

        const char* label_fill   = label.attribute("fill").value();
        const double label_font  = label.attribute("font_px").as_double();

        const char* sig_color    = signal.attribute("color").value();
        const double sig_width   = signal.attribute("width").as_double();
        const double sig_tip_r   = signal.attribute("tip_radius").as_double();
        const char* sig_lbl_fill = siglbl.attribute("fill").value();
        const double sig_lbl_font= siglbl.attribute("font_px").as_double();

        (void)node_radius; // applied inline in body

        fmt::memory_buffer css;
        fmt::format_to(std::back_inserter(css),
            "  svg{{ {1};{0} }}\n",
            (*svg_bg ? fmt::format(" background:{};", svg_bg) : std::string{}),
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
        auto cfg = config->child("config");
        const double node_radius  = cfg.child("node").attribute("radius").as_double();
        const double sig_lbl_font = cfg.child("signal").child("label").attribute("font_px").as_double();

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
        // logger->trace("rendering with config (xml)");
        kainjow::mustache::data ctx;
        ctx.set("width",  fmt::format("{}", width));
        ctx.set("height", fmt::format("{}", height));
        ctx.set("title",  title);
        ctx.set("metadata", metadata); // template must use {{{metadata}}}
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

// ------------------ Config I/O: load / loads / dump / dumps (XML) --------------

GraphSvg& GraphSvg::loadConfig(const std::string& file_path) {
    pugi::xml_document incoming;
    auto res = incoming.load_file(file_path.c_str());
    if (!res) {
        pimpl->logger->error("loadConfig: parse failed: {}", res.description());
        throw std::runtime_error("XML parse failed");
    }
    auto src = incoming.child("config");
    if (!src) {
        pimpl->logger->error("loadConfig: <config> root not found in '{}'", file_path);
        throw std::runtime_error("XML root <config> not found");
    }

    auto dst = pimpl->config->child("config");
    Impl::xml_deep_merge(dst, src);

    // sync mirrors
    auto scale = dst.child("scale");
    pimpl->node_space_scale = scale.attribute("node_space").as_double();
    pimpl->signal_scale     = scale.attribute("signal").as_double();

    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::loadsConfig(const std::string& xml_text) {
    pugi::xml_document incoming;
    auto res = incoming.load_string(xml_text.c_str());
    if (!res) {
        pimpl->logger->error("loadsConfig: parse failed: {}", res.description());
        throw std::runtime_error("XML parse failed");
    }
    auto src = incoming.child("config");
    if (!src) {
        pimpl->logger->error("loadsConfig: <config> root not found in string");
        throw std::runtime_error("XML root <config> not found");
    }

    auto dst = pimpl->config->child("config");
    Impl::xml_deep_merge(dst, src);

    auto scale = dst.child("scale");
    pimpl->node_space_scale = scale.attribute("node_space").as_double();
    pimpl->signal_scale     = scale.attribute("signal").as_double();

    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::dumpConfig(const std::string& file_path) const {
    if (!pimpl->config->save_file(file_path.c_str(), PUGIXML_TEXT("  "))) {
        pimpl->logger->error("dumpConfig: save failed '{}'", file_path);
        throw std::runtime_error("dumpConfig: save failed");
    }
    pimpl->logger->info("Saved XML config to {}", file_path);
    return const_cast<GraphSvg&>(*this);
}

std::string GraphSvg::dumpsConfig() const {
    std::ostringstream oss;
    pimpl->config->save(oss, PUGIXML_TEXT("  "));
    return oss.str();
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
    // assumes path exists
    pimpl->config->child("config").child("scale").attribute("node_space").set_value(v);
    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::setSignalScale(double v) {
    pimpl->signal_scale = v;
    // assumes path exists
    pimpl->config->child("config").child("scale").attribute("signal").set_value(v);
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

    const std::string metadata = pimpl->build_metadata(); // embeds <metadata><plotinfo><config>...</config>
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
    pimpl->config->child("config").child("svg").attribute("css").set_value(css.c_str());
    pimpl->dirty_svg = true;
    return *this;
}

GraphSvg& GraphSvg::setSvgBackground(const std::string& bg) {
    pimpl->config->child("config").child("svg").attribute("background").set_value(bg.c_str());
    pimpl->dirty_svg = true;
    return *this;
}

} // namespace gsp
