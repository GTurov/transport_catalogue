#include "map_renderer.h"

#include <fstream> // debug
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

    // Line
    int col = 0;
    for (transport::Route* r: routes_to_draw) {
        if (r->stopsCount() == 0) {
            break;
        }
        svg::Polyline route_line = svg::Polyline()
                .SetStrokeColor(settings_.color_palette[col])
                .SetStrokeWidth(settings_.line_width)
                .SetFillColor({})
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                ;
        ++col;
        if(col == (int)settings_.color_palette.size()) {
            col = 0;
        }
        for (auto it = r->stops().begin(); it != r->stops().end(); ++it) {
            //std::cerr<<(*it)->name()<<" "s;
            route_line.AddPoint(scaler((*it)->place()));
        }
        if (!r->isCycled()) {
            for (auto r_it = r->stops().rbegin(); r_it != r->stops().rend(); ++r_it) {
                route_line.AddPoint(scaler((*r_it)->place()));
            }
        }
        svg_document.Add(route_line);
    }

    // Route labels
    col = 0;
    for (transport::Route* r: routes_to_draw) {
        if (r->stopsCount() == 0) {
            break;
        }

        svg::Text route_label_underlayer = svg::Text()
                .SetPosition(scaler((*r->stops().begin())->place()))
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Veranda")
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
                .SetFontFamily("Veranda")
                .SetFontWeight("bold"s)
                .SetData(r->name())
                .SetStrokeColor(settings_.color_palette[col])
                .SetFillColor(settings_.color_palette[col])
                ;
        svg_document.Add(route_label);

        if (!r->isCycled()) {
            svg::Text stop_label_underlayer = svg::Text()
                    .SetPosition(scaler((*r->stops().rbegin())->place()))
                    .SetOffset(settings_.bus_label_offset)
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Veranda")
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
                    .SetFontFamily("Veranda")
                    .SetFontWeight("bold"s)
                    .SetData(r->name())
                    .SetStrokeColor(settings_.color_palette[col])
                    .SetFillColor(settings_.color_palette[col])
                    ;
            svg_document.Add(stop_label);
            ++col;
            if(col == (int)settings_.color_palette.size()) {
                col = 0;
            }
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
                .SetFontFamily("Veranda")
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
                .SetFontFamily("Veranda")
                .SetData(s->name())
                .SetFillColor("black")
                ;
        svg_document.Add(stop_label);
    }

//    std::ofstream svg_file("map.svg"s);
//    svg_document.Render(svg_file);
//    svg_file.close();


    std::string buffer;
    std::stringstream ss(buffer);
    //std::stringstream ss();
    //std::ostringstream os;

//    std::ostringstream ss(std::ios_base::out);
    svg_document.Render(ss);

    return ss.str();
}
