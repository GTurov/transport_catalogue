#include "map_renderer.h"

//#define SVG_TO_FILE
//#define SVG_COMPARE

#include <cassert>

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

    // Line
    int color_index = 0;
    for (transport::Route* r: routes_to_draw) {
        if (r->stopsCount() == 0) {
            break;
        }
        svg::Polyline route_line = svg::Polyline()
                .SetStrokeColor(settings_.color_palette[color_index])
                .SetStrokeWidth(settings_.line_width)
                .SetFillColor({})
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                ;
        ++color_index;
        if(color_index == (int)settings_.color_palette.size()) {
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
            break;
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

        if (!r->isCycled()) {
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
            ++color_index;
            if(color_index == (int)settings_.color_palette.size()) {
                color_index = 0;
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

#ifdef SVG_COMPARE
    std::string s = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"s +
    "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"s +
    "  <polyline points=\"99.2283,329.5 50,232.18 99.2283,329.5\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n"s +
    "  <polyline points=\"550,190.051 279.22,50 333.61,269.08 550,190.051\" fill=\"none\" stroke=\"rgb(255,160,0)\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n"s +
    "  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"99.2283\" y=\"329.5\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n"s +
    "  <text fill=\"green\" x=\"99.2283\" y=\"329.5\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n"s +
    "  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"232.18\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n"s +
    "  <text fill=\"green\" x=\"50\" y=\"232.18\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n"s +
    "  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"550\" y=\"190.051\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n"s +
    "  <text fill=\"rgb(255,160,0)\" x=\"550\" y=\"190.051\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n"s +
    "  <circle cx=\"99.2283\" cy=\"329.5\" r=\"5\" fill=\"white\"/>\n"s +
    "  <circle cx=\"50\" cy=\"232.18\" r=\"5\" fill=\"white\"/>\n"s +
    "  <circle cx=\"333.61\" cy=\"269.08\" r=\"5\" fill=\"white\"/>\n"s +
    "  <circle cx=\"550\" cy=\"190.051\" r=\"5\" fill=\"white\"/>\n"s +
    "  <circle cx=\"279.22\" cy=\"50\" r=\"5\" fill=\"white\"/>\n"s +
    "  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"99.2283\" y=\"329.5\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n"s +
    "  <text fill=\"black\" x=\"99.2283\" y=\"329.5\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n"s +
    "  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"232.18\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n"s +
    "  <text fill=\"black\" x=\"50\" y=\"232.18\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n"s +
    "  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"333.61\" y=\"269.08\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Улица Докучаева</text>\n"s +
    "  <text fill=\"black\" x=\"333.61\" y=\"269.08\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Улица Докучаева</text>\n"s +
    "  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"550\" y=\"190.051\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Улица Лизы Чайкиной</text>\n"s +
    "  <text fill=\"black\" x=\"550\" y=\"190.051\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Улица Лизы Чайкиной</text>\n"s +
    "  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"279.22\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Электросети</text>\n"s +
    "  <text fill=\"black\" x=\"279.22\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Электросети</text>\n"s +
    "</svg>"s;

    std::istringstream got(ss.str());
    std::istringstream need(s);
    std::string lhs, rhs;
    int str_num = 0;
    while (getline(got, lhs) && getline(need, rhs)) {
        if(lhs != rhs) {
            std::cerr << ++str_num << std::endl;
            std::cerr<<lhs<<std::endl<<rhs<<std::endl;
            for(int i =0; i < (int)lhs.size(); ++i) {
                std::cerr<<(lhs[i]==rhs[i]?' ':'^');
            }
            assert(false);
        }
    }


    assert(s.size() == ss.str().size());
#endif

    return ss.str();
}
