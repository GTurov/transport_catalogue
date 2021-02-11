#pragma once

#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <iostream>
#include <string>

json::Node makeStopAnswer(int request_id, transport::Stop::Info data);
json::Node makeRouteAnswer(int request_id, transport::Route::Info data);

svg::Color nodeToColor(const json::Node& n);

struct renderSettings {
    double width = 0;
    double height = 0;

    double padding = 0;
    double line_width = 0;
    double stop_radius = 0;

    double bus_label_font_size = 0;
    std::array<double,2> bus_label_offset = {0,0};

    double stop_label_font_size = 0;
    std::array<double,2> stop_label_offset = {0,0};

    svg::Color underlayer_color = {};
    double underlayer_width = 0;

    std::vector<svg::Color> color_palette = {};
};

enum class request_type {
    REQUEST_STOP, REQUEST_BUS, REQUEST_MAP
};

struct request {
    int id;
    request_type type;
    std::string name;
};

struct stopAnswer {
    int id = 0;
    transport::Stop::Info answer;
    std::string error = "";
};

class json_reader
{
public:
    json_reader(transport::Catalogue& catalogue)
        :catalogue_(catalogue) {}
    void process_queries(std::istream& in = std::cin, std::ostream& out = std::cout);
private:

private:
    transport::Catalogue& catalogue_;
};

