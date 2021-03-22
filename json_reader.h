#pragma once


#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <iostream>
#include <optional>
#include <string>


svg::Color nodeToColor(const json::Node& node);

class JsonReader {
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
public:
    JsonReader() = default;
    void processQueries(std::istream& in = std::cin, std::ostream& out = std::cout);

private:
    static json::Node makeNotFoundAnswer(int requestId);
    static json::Node makeStopAnswer(int requestId, std::string_view name, const transport::Catalogue& catalogue);
    static json::Node makeRouteAnswer(int requestId, std::string_view name, const transport::Catalogue& catalogue);
    static json::Node makeMapAnswer(int requestId, const MapRenderer& renderer);
    static json::Node makePathAnswer(int requestId, std::string_view from, std::string_view to, const transport::RouteFinder& finder);
    static RenderSettings parseRenderSettings(const json::Node& node);
    static RoutingSettings parseRoutingSettings(const json::Node& node);
    static std::vector<Request> parseStatRequests(const json::Node& node);
    static void fillDataBase(transport::Catalogue& catalogue,
                      const std::vector<const json::Node*>& stopNodes,
                      const std::vector<const json::Node*>& routeNodes);
    static json::Node prepareAnswers(std::vector<Request>& requests,
                              transport::Catalogue& catalogue,
                              MapRenderer& renderer,
                              transport::RouteFinder& finder);
};

