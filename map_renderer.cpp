#include "map_renderer.h"

//#define SVG_TO_FILE

#ifdef SVG_TO_FILE
#include <fstream> // debug
#endif
#include <unordered_set>
#include <sstream>

std::string map_renderer::render() const {
    using namespace std::literals;

    const transport::RouteSet routes_to_draw = catalogue_.allRoutes();
    transport::StopSet stops_to_draw;
    for (transport::Route* r: routes_to_draw) {
        for (transport::Stop* s: r->stops()) {
            stops_to_draw.insert(s);
        }
    }

    MapScaler scaler(stops_to_draw.begin(), stops_to_draw.end(),
                     settings_.width, settings_.height, settings_.padding);

    svg::Document svg_document;

    // Lines
    int color_index = 0;
    for (transport::Route* r: routes_to_draw) {
        if (r->stopsCount() < 2) {
            continue;
        }
        svg::Polyline route_line = svg::Polyline()
                .SetStrokeColor(settings_.color_palette[color_index])
                .SetStrokeWidth(settings_.line_width)
                .SetFillColor({})
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                ;
        ++color_index;
        if (color_index == (int)settings_.color_palette.size()) {
            color_index = 0;
        }
        for (auto it = r->stops().begin(); it != r->stops().end(); ++it) {
            route_line.AddPoint(scaler((*it)->place()));
        }
        if (!r->isCycled()) {
            for (auto r_it = ++r->stops().rbegin(); r_it != r->stops().rend(); ++r_it) {
                route_line.AddPoint(scaler((*r_it)->place()));
            }
        }
        svg_document.Add(route_line);
    }

    // Route labels
    color_index = 0;
    for (transport::Route* r: routes_to_draw) {
        if (r->stopsCount() == 0) {
            continue;
        }

        svg::Text route_label_underlayer = svg::Text()
                .SetPosition(scaler((*r->stops().begin())->place()))
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold"s)
                .SetData(r->name())
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                ;
        svg_document.Add(route_label_underlayer);

        svg::Text route_label = svg::Text()
                .SetPosition(scaler((*r->stops().begin())->place()))
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana")
                .SetFontWeight("bold"s)
                .SetData(r->name())
                .SetFillColor(settings_.color_palette[color_index])
                ;
        svg_document.Add(route_label);

        if (!r->isCycled() && r->stops().size()>1 && (*r->stops().begin() != *r->stops().rbegin())) {
            svg::Text stop_label_underlayer = svg::Text()
                    .SetPosition(scaler((*r->stops().rbegin())->place()))
                    .SetOffset(settings_.bus_label_offset)
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold"s)
                    .SetData(r->name())
                    .SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                    ;
            svg_document.Add(stop_label_underlayer);

            svg::Text stop_label = svg::Text()
                    .SetPosition(scaler((*r->stops().rbegin())->place()))
                    .SetOffset(settings_.bus_label_offset)
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetFontWeight("bold"s)
                    .SetData(r->name())
                    .SetFillColor(settings_.color_palette[color_index])
                    ;
            svg_document.Add(stop_label);
        }
        ++color_index;
        if (color_index == (int)settings_.color_palette.size()) {
            color_index = 0;
        }
    }

    // Stop rounds
    for (transport::Stop* s: stops_to_draw) {
        svg::Circle stop_round = svg::Circle()
                .SetCenter(scaler(s->place()))
                .SetRadius(settings_.stop_radius)
                .SetFillColor("white"s)
                ;
        svg_document.Add(stop_round);
    }

    //Stop labels
    for (transport::Stop* s: stops_to_draw) {
        svg::Text stop_label_underlayer = svg::Text()
                .SetPosition(scaler(s->place()))
                .SetOffset(settings_.stop_label_offset)
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(s->name())
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                ;
        svg_document.Add(stop_label_underlayer);

        svg::Text stop_label = svg::Text()
                .SetPosition(scaler(s->place()))
                .SetOffset(settings_.stop_label_offset)
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana")
                .SetData(s->name())
                .SetFillColor("black")
                ;
        svg_document.Add(stop_label);
    }

#ifdef SVG_TO_FILE
    std::ofstream svg_file("map.svg"s);
    svg_document.Render(svg_file);
    svg_file.close();
#endif
    std::string buffer;
    std::stringstream ss(buffer);
    svg_document.Render(ss);

    return ss.str();
}
