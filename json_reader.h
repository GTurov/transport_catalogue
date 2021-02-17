#pragma once

#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <iostream>
#include <string>

json::Node makeStopAnswer(int request_id, const transport::Stop::Info& data);
json::Node makeRouteAnswer(int request_id, const transport::Route::Info& data);

svg::Color nodeToColor(const json::Node& n);

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

class json_reader {
public:
    json_reader(transport::Catalogue& catalogue)
        :catalogue_(catalogue) {}
    void process_queries(std::istream& in = std::cin, std::ostream& out = std::cout);

private:
    transport::Catalogue& catalogue_;
    renderSettings rs_;
};

