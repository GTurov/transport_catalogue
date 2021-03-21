#pragma once


#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <iostream>
#include <optional>
#include <string>

json::Node makeStopAnswer(int requestId, const transport::Stop::Info& data);
json::Node makeRouteAnswer(int requestId, const transport::Route::Info& data);
json::Node makePathAnswer(int requestId, const std::vector<transport::TripItem>& data);

svg::Color nodeToColor(const json::Node& n);

struct RoutingSettings {
    int busWaitTime;
    double busVelocity;
};

enum class REQUEST_TYPE {
    STOP, BUS, MAP, ROUTE
};

struct Request {
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
    RenderSettings renderSettings_;
    RoutingSettings routingSettings_;
};

