#include <fstream>
#include <sstream>
#include <cmath>
#include <mustache.hpp>

std::string render_svg(
    const std::vector<std::tuple<int, int>> &edges,
    const std::vector<std::pair<double, double>> &coords,
    const std::vector<double> &signals
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

    kainjow::mustache::data ctx;
    ctx.set("title", "Network");
    ctx.set("width", std::to_string((int)(max_x - min_x)));
    ctx.set("height", std::to_string((int)(max_y - min_y)));
    ctx.set("metadata", "<generator name=\"libgsp\" author=\"Mohammad Raziei\" />");

    ctx["edges"] = kainjow::mustache::data::type::list;
    ctx["nodes"] = kainjow::mustache::data::type::list;
    auto &edges_data = ctx["edges"];
    auto &nodes_data = ctx["nodes"];

    // اضافه کردن یال‌ها
    for (auto &[src, tgt] : edges) {
        auto &e = edges_data.push_back(kainjow::mustache::data::type::object);
        auto [x1, y1] = coords[src];
        auto [x2, y2] = coords[tgt];
        e.set("x1", std::to_string(x1 * scale - min_x));
        e.set("y1", std::to_string(-y1 * scale + max_y));
        e.set("x2", std::to_string(x2 * scale - min_x));
        e.set("y2", std::to_string(-y2 * scale + max_y));
        e.set("src", std::to_string(src));
        e.set("tgt", std::to_string(tgt));
    }

    for (size_t i = 0; i < coords.size(); ++i) {
        auto &n = nodes_data.push_back(kainjow::mustache::data::type::object);
        double x = coords[i].first * scale - min_x;
        double y = -coords[i].second * scale + max_y;
        double signal = signals[i];

        n.set("x", std::to_string(x));
        n.set("y", std::to_string(y));
        n.set("label", std::to_string(i));
        n.set("signal", std::to_string(signal));

        if (signal != 0) {
            n.set("has_signal", "true");
            double value = -signal * scale;
            n.set("x2", std::to_string(x));
            n.set("y2", std::to_string(y + value));
            n.set("text_y", std::to_string(y + value + (value < 0 ? -font_size : font_size)));
        }
    }

    std::ifstream template_file("graph.svg.mustache");
    std::stringstream buffer;
    buffer << template_file.rdbuf();
    kainjow::mustache::mustache tmpl(buffer.str());

    return tmpl.render(ctx);
}