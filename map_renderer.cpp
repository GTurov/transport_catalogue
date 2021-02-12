#include "map_renderer.h"

#include <fstream> // debug
#include <unordered_set>

std::string map_renderer::render() const {
    using namespace std::literals;

    const transport::RouteSet routes_to_draw = catalogue_.allRoutes();
    std::unordered_set<transport::Stop*> stops_to_draw;
    for (transport::Route* r: routes_to_draw) {
        for (transport::Stop* s: r->stops()) {
            stops_to_draw.insert(s);
        }
    }

    MapScaler scaler(stops_to_draw.begin(), stops_to_draw.end(),
                     settings_.width, settings_.height, settings_.padding);

    svg::Document svg_document;


    // Line
    for (transport::Route* r: routes_to_draw) {
        svg::Polyline route_line = svg::Polyline()
                .SetStrokeColor("red") //TODO
                .SetStrokeWidth(settings_.line_width)
                .SetFillColor({})
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        for (transport::Stop* s: r->stops()) {
            route_line.AddPoint(scaler(s->place()));
        }
        svg_document.Add(route_line);
    }

    // Route labels
    for (transport::Route* r: routes_to_draw) {
        for (transport::Stop* s: r->stops()) {
            svg::Circle stop_round = svg::Circle()
                    .SetCenter(scaler(s->place()))
                    .SetRadius(settings_.stop_radius)
                    .SetFillColor("white"s);
            svg_document.Add(stop_round);
        }
    }

    // Stop rounds



    std::ofstream svg_file("map.svg"s);
    svg_document.Render(svg_file);
    svg_file.close();

    return "svg_document"s;
}
