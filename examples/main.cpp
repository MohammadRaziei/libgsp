//
// Created by mohammad on 5/20/23.
//
#include <iostream>
#include <memory>
#include <filesystem>


#include <spdlog/spdlog.h>

#include <mustache.hpp>
#include <lunasvg.h>

#include "libgsp/graph/graph.h"
#include "libgsp/graph/graphsignal.h"



#include "libgsp/io/file.h"
#include "libgsp/utils/utils.h"
#include "common.h"

namespace fs = std::filesystem;


std::string render_svg(
    const std::vector<gsp::Edge> &edges,
    const std::vector<std::pair<double, double>> &coords,
    const std::vector<double> &signals, 
    const std::map<std::string, std::string> &options = {}
) {
    const double signal_scale = map_get(options, "signal_scale", 1);
    int node_space_scale = map_get(options, "node_space_scale", 1);
    const int signal_font_size = map_get(options, "signal_font_size", 12);
    std::string title = map_get<std::string>(options, "title", "");


    double min_x = 1e9, max_x = -1e9, min_y = 1e9, max_y = -1e9;

    for (size_t i = 0; i < coords.size(); ++i) {
        double x = coords[i].first * signal_scale;
        double y = coords[i].second * signal_scale;
        double sig = signals[i] * signal_scale;
        double x_sig = x;
        double y_sig = y + sig;

        min_x = std::min(min_x, std::min(x, x_sig) - 20);
        max_x = std::max(max_x, std::max(x, x_sig) + 20);
        min_y = std::min(min_y, std::min(y, y_sig) - 20);
        max_y = std::max(max_y, std::max(y, y_sig) + 20);
    }

    char buf[32];

    kainjow::mustache::data ctx;
    ctx.set("title", title);
    ctx.set("width", std::to_string((int)(max_x - min_x)));
    ctx.set("height", std::to_string((int)(max_y - min_y)));
    ctx.set("metadata", "<generator name=\"libgsp\" author=\"Mohammad Raziei\" />");

    ctx["edges"] = kainjow::mustache::data::type::list;
    ctx["nodes"] = kainjow::mustache::data::type::list;
    auto &edges_data = ctx["edges"];
    auto &nodes_data = ctx["nodes"];

    for (auto &[src, tgt, w] : edges) {
        kainjow::mustache::data e(kainjow::mustache::data::type::object);
        auto [x1, y1] = coords[src];
        auto [x2, y2] = coords[tgt];
        e.set("x1", std::to_string(x1 * signal_scale - min_x));
        e.set("y1", std::to_string(-y1 * signal_scale + max_y));
        e.set("x2", std::to_string(x2 * signal_scale - min_x));
        e.set("y2", std::to_string(-y2 * signal_scale + max_y));
        e.set("src", std::to_string(src));
        e.set("tgt", std::to_string(tgt));
        edges_data.push_back(e);
    }

    for (size_t i = 0; i < coords.size(); ++i) {
        kainjow::mustache::data n(kainjow::mustache::data::type::object);
        double x = coords[i].first * signal_scale - min_x;
        double y = -coords[i].second * signal_scale + max_y;
        double signal = signals[i];

        n.set("x", std::to_string(x));
        n.set("y", std::to_string(y));
        n.set("label", std::to_string(i));
        std::sprintf(buf, "%g", signal);
        n.set("signal", buf);

        if (signal != 0) {
            n.set("has_signal", "true");
            double value = -signal * signal_scale;
            n.set("x2", std::to_string(x));
            n.set("y2", std::to_string(y + value));
            n.set("text_y", std::to_string(y + value + (value < 0 ? -signal_font_size : signal_font_size)));
        }
        nodes_data.push_back(n);
    }

    fs::path current_path = fs::path(__FILE__).parent_path();
    fs::path template_file_name = fs::absolute(current_path.parent_path() / "include/io/templates/svg/graph.svg.mustache");

    const std::string template_file = readFile(template_file_name.string());

    kainjow::mustache::mustache tmpl(template_file);

    return tmpl.render(ctx);
}


int svg2png(const std::string& svg_content){
    auto document = lunasvg::Document::loadFromData(svg_content);
    if (!document) {
        std::cerr << "Failed to load SVG file!" << std::endl;
        return -1;
    }

    const float documentWidth = document->width();
    const float documentHeight = document->height();

    std::cout << "width = " << documentWidth << ", height = " << documentHeight << std::endl;

    const float scale = 5;

    // Render to bitmap
    const std::shared_ptr<lunasvg::Bitmap> bitmap = std::make_shared<lunasvg::Bitmap>(document->renderToBitmap(documentWidth*scale, documentHeight*scale));
    if (bitmap && bitmap->isNull()) {
        std::cerr << "Failed to render SVG to bitmap!" << std::endl;
        return -1;
    }

    // Save as PNG
    if (!bitmap->writeToPng("graph.png")) {
        std::cerr << "Failed to save PNG file!" << std::endl;
        return -1;
    }

    return 0;
}


int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");


    spdlog::info("Hello, world!");


    std::vector<gsp::Edge> edges = {{0,1},{0,2},{1,2},{2,3}};
    std::vector<std::pair<double,double>> coords_vec = {{0,0}, {2,0}, {1,-1}, {3,-1}};
    std::vector<double> signal_vec = {-0.04, 0.31, 0.06, 0.39};

    const uint32_t num_nodes = 4;
    double* coords_ptr = (double *) coords_vec.data();
    alglib::real_2d_array coords;
    coords.attach_to_ptr(num_nodes,2, coords_ptr);

    std::cout << coords.tostring(0) << std::endl;

    gsp::DenseGraph graph(num_nodes);
    graph.setCoords(coords_vec);
    graph.setWeights(edges);

    std::cout << graph.weights.tostring(0) << std::endl;

    alglib::real_1d_array signal;
    signal.attach_to_ptr(num_nodes, signal_vec.data());
    gsp::GraphSignal graph_signal(graph, signal);

    printf("\ngood bye :)\n");


    std::map<std::string, std::string> options = {
        {"signal_scale", "100"},
        {"node_space_scale", "100"},
        {"signal_font_size", "6"},
        {"title", "Network"},
    };

    std::string svg = render_svg(edges, coords_vec, signal_vec, options);

    writeFile("graph.svg", svg);
    spdlog::info("SVG file written to graph.svg");

    return svg2png(svg);
}