#include "map_renderer.h"

#include <fstream> // debug

svg::Point map_renderer::coordinatesToPoint(const Coordinates& place) const {

    return {};
}


std::string map_renderer::render() const {
    using namespace std::literals;

    const transport::RouteSet routes_to_draw = catalogue_.allRoutes();
    transport::StopSet stops_to_draw;
    for (transport::Route* r: routes_to_draw) {
        for (transport::Stop* s: r->stops()) {
            stops_to_draw.insert(s);
        }

    }


    std::string svg_image;

    // готовим изображение

    std::ofstream svg_file("map.svg"s);
    svg_file << svg_image;
    svg_file.close();

    return svg_image;
}
