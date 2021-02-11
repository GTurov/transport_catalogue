#pragma once

#include <array>
#include <vector>
#include <string>

#include "svg.h"
#include "transport_catalogue.h"

struct renderSettings {
    double width = 0;
    double height = 0;

    double padding = 0;
    double line_width = 0;
    double stop_radius = 0;

    double bus_label_font_size = 0;
    std::array<double,2> bus_label_offset = {0,0};
    // point

    double stop_label_font_size = 0;
    std::array<double,2> stop_label_offset = {0,0};

    svg::Color underlayer_color = {};
    double underlayer_width = 0;

    std::vector<svg::Color> color_palette = {};
};

class map_renderer
{
public:
    map_renderer(transport::Catalogue& catalogue)
        :catalogue_(catalogue) {}
    std::string render_map(const renderSettings& settings) const;
private:
    transport::Catalogue& catalogue_;
};

