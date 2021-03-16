#pragma once


#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <iostream>
#include <optional>
#include <string>

json::Node makeStopAnswer(int request_id, const transport::Stop::Info& data);
json::Node makeRouteAnswer(int request_id, const transport::Route::Info& data);
json::Node makePathAnswer(int request_id, const std::vector<transport::TripItem>& data);

svg::Color nodeToColor(const json::Node& n);

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

enum class REQUEST_TYPE {
    STOP, BUS, MAP, ROUTE
};

struct request {
    int id;
    REQUEST_TYPE type;
    std::string name;
    std::string from;
    std::string to;
};

class JsonReader {
public:
    JsonReader(transport::Catalogue& catalogue)
        :catalogue_(catalogue) {}
    void processQueries(std::istream& in = std::cin, std::ostream& out = std::cout);

private:
    transport::Catalogue& catalogue_;
    renderSettings rs_;
    RoutingSettings rrs_;
};

