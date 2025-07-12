//
// Created by mohammad on 5/20/23.
//
#include <iostream>
#include <memory> 

#include <aixlog.hpp>
#include <mustache.hpp>

#include "io/file.h"
#include "utils/utils.h"
#include "common.h"

#undef LOGGER
#define LOGGER(level) LOG(level) << TAG("main") << " "



std::string render_svg(
    const std::vector<std::tuple<int, int>> &edges,
    const std::vector<std::pair<double, double>> &coords,
    const std::vector<double> &signals, 
    const std::map<std::string, std::string> &options = {}
) {
    const double scale = 100.0;
    const int font_size = 6;
    double min_x = 1e9, max_x = -1e9, min_y = 1e9, max_y = -1e9;

    for (size_t i = 0; i < coords.size(); ++i) {
        double x = coords[i].first * scale;
        double y = coords[i].second * scale;
        double sig = signals[i] * scale;
        double x_sig = x;
        double y_sig = y + sig;

        min_x = std::min(min_x, std::min(x, x_sig) - 20);
        max_x = std::max(max_x, std::max(x, x_sig) + 20);
        min_y = std::min(min_y, std::min(y, y_sig) - 20);
        max_y = std::max(max_y, std::max(y, y_sig) + 20);
    }

    char buf[32];

    kainjow::mustache::data ctx;
    ctx.set("title", "Network");
    ctx.set("width", std::to_string((int)(max_x - min_x)));
    ctx.set("height", std::to_string((int)(max_y - min_y)));
    ctx.set("metadata", "<generator name=\"libgsp\" author=\"Mohammad Raziei\" />");

    ctx["edges"] = kainjow::mustache::data::type::list;
    ctx["nodes"] = kainjow::mustache::data::type::list;
    auto &edges_data = ctx["edges"];
    auto &nodes_data = ctx["nodes"];

    for (auto &[src, tgt] : edges) {
        kainjow::mustache::data e(kainjow::mustache::data::type::object);
        auto [x1, y1] = coords[src];
        auto [x2, y2] = coords[tgt];
        e.set("x1", std::to_string(x1 * scale - min_x));
        e.set("y1", std::to_string(-y1 * scale + max_y));
        e.set("x2", std::to_string(x2 * scale - min_x));
        e.set("y2", std::to_string(-y2 * scale + max_y));
        e.set("src", std::to_string(src));
        e.set("tgt", std::to_string(tgt));
        edges_data.push_back(e);
    }

    for (size_t i = 0; i < coords.size(); ++i) {
        kainjow::mustache::data n(kainjow::mustache::data::type::object);
        double x = coords[i].first * scale - min_x;
        double y = -coords[i].second * scale + max_y;
        double signal = signals[i];

        n.set("x", std::to_string(x));
        n.set("y", std::to_string(y));
        n.set("label", std::to_string(i));
        std::sprintf(buf, "%g", signal);
        n.set("signal", buf);

        if (signal != 0) {
            n.set("has_signal", "true");
            double value = -signal * scale;
            n.set("x2", std::to_string(x));
            n.set("y2", std::to_string(y + value));
            n.set("text_y", std::to_string(y + value + (value < 0 ? -font_size : font_size)));
        }
        nodes_data.push_back(n);
    }

    const std::string template_file = readFile("includes/io/templates/svg/graph.svg.mustache");

    kainjow::mustache::mustache tmpl(template_file);

    return tmpl.render(ctx);
}


int main(int argc, char** argv){
    AixLog::Log::init(
        {
            std::make_shared<AixLog::SinkCout>(AixLog::Severity::trace, "%Y-%m-%d %H-%M-%S.#ms [#severity] (#tag) #message"),

        }
    );

    LOGGER(INFO) << "Hello, world!";



    printf("\ngood bye :)\n");




    std::vector<std::tuple<int, int>> edges = {{0,1},{0,2},{1,2},{2,3}};
    std::vector<std::pair<double,double>> coords = {{0,0}, {2,0}, {1,-1}, {3,-1}};
    std::vector<double> signals = {-0.04, 0.31, 0.06, 0.39};


    std::map<std::string, std::string> options = {
        {"signal_scale", "100"},
        {"node_space_scale", "100"},
        {"signal_font_size", "6"},
        {"title", "Network"},
    };

    std::string svg = render_svg(edges, coords, signals, options);


    writeFile("output.svg", svg);
    LOGGER(INFO) << "SVG file written to output.svg";		
    return 0;
}



