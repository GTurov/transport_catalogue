#include "json_reader.h"

#include "json_builder.h"

#include <vector>
#include <optional>

using namespace std::literals;



svg::Color nodeToColor(const json::Node& node) {
    if (node.isString()) {
        return svg::Color(node.asString());
    }
    if (node.isArray()) {
        const json::Array& color = node.asArray();
        if (color.size() == 3) {
            return svg::Color(
                        svg::Rgb(color[0].asInt(),color[1].asInt(),color[2].asInt()));
        }

        if (color.size() == 4) {
            return svg::Color(
                        svg::Rgba(color[0].asInt(),color[1].asInt(),color[2].asInt(),
                    color[3].asDouble()));
        }
    }
    throw std::exception();
}

void JsonReader::processQueries(std::istream& in, std::ostream& out) {
    using namespace json;

    const Document rawRequests = json::load(in);

    // Base requests
    const Node& baseRequests = rawRequests.getRoot().asDict().at("base_requests");
    std::vector<const Node*> stopNodes;
    std::vector<const Node*> busNodes;
    for (const Node& n: baseRequests.asArray()) {
        if (n.asDict().at("type"s).asString() == "Stop"s) {
            stopNodes.push_back(&n);
            continue;
        }
        if (n.asDict().at("type"s).asString() == "Bus"s) {
            busNodes.push_back(&n);
            continue;
        }
        throw json::ParsingError("Base request parsing error"s);
    }
    transport::Catalogue catalogue;
    this->fillDataBase(catalogue, stopNodes, busNodes);

    // Render settings
    RenderSettings renderSettings;
    if (rawRequests.getRoot().asDict().find("render_settings") != rawRequests.getRoot().asDict().end()) {
        renderSettings = parseRenderSettings(rawRequests.getRoot().asDict().at("render_settings"));
    }
    MapRenderer renderer(catalogue, renderSettings);

    // Routing settings
    RoutingSettings routingSettings;
    if (rawRequests.getRoot().asDict().find("routing_settings") != rawRequests.getRoot().asDict().end()) {
        routingSettings = parseRoutingSettings(rawRequests.getRoot().asDict().at("routing_settings"));
    }
    transport::RouteFinder finder(catalogue, routingSettings.busWaitTime, routingSettings.busVelocity);

    // Stat requests
    std::vector<Request> statRequests = parseStatRequests(rawRequests.getRoot().asDict().at("stat_requests"));

    // Answers
    out << this->prepareAnswers(statRequests, catalogue, renderer, finder);
}

json::Node JsonReader::makeNotFoundAnswer(int requestId) {
    return json::Builder{}
    .startDict()
    .key("request_id"s).value(requestId)
    .key("error_message"s).value("not found"s)
    .endDict()
    .build();
}


json::Node JsonReader::makeStopAnswer(int requestId, std::string_view name, const transport::Catalogue& catalogue) {
    if (auto stopInfo = catalogue.stopInfo(name); stopInfo) {
        json::Builder builder;
        builder.startArray();
        for (transport::Route* r: stopInfo.value().routes) {
            builder.value(r->name());
        }
        json::Node x = builder.endArray().build();
        return json::Builder{}
        .startDict()
        .key("request_id"s).value(requestId)
        .key("buses"s).value(x.asArray())
        .endDict().build();
    } else {
        return makeNotFoundAnswer(requestId);
    }
}

json::Node JsonReader::makeRouteAnswer(int requestId, std::string_view name, const transport::Catalogue& catalogue) {
    if (auto routeInfo = catalogue.routeInfo(name); routeInfo) {
        return json::Builder{}
        .startDict()
        .key("request_id"s).value(requestId)
        .key("route_length"s).value(routeInfo.value().length)
        .key("stop_count"s).value(routeInfo.value().stopCount)
        .key("unique_stop_count"s).value(routeInfo.value().uniqueStopCount)
        .key("curvature"s).value(routeInfo.value().curvature)
        .endDict()
        .build()
        ;
    } else {
        return makeNotFoundAnswer(requestId);
    }
}

json::Node JsonReader::makeMapAnswer(int requestId, const MapRenderer& renderer) {
    return json::Builder{}
    .startDict()
    .key("request_id"s).value(requestId)
    .key("map"s).value(renderer.render())
    .endDict()
    .build();
}

json::Node JsonReader::makePathAnswer(int requestId, std::string_view from, std::string_view to, const transport::RouteFinder& finder) {
    if (auto pathInfo = finder.findRoute(from, to); pathInfo) {
        json::Builder builder;
        builder.startArray();
        double totalTime = 0;
        for (const auto& item: pathInfo.value()) {
            totalTime += (item->spending.waitTime + item->spending.tripTime);
            builder.startDict()
                    .key("type"s).value("Wait"s)
                    .key("stop_name"s).value(item->from->name())
                    .key("time"s).value(item->spending.waitTime / 60)
                    .endDict()

                    .startDict()
                    .key("type"s).value("Bus"s)
                    .key("bus"s).value(item->bus->name())
                    .key("span_count"s).value(item->spending.stopCount)
                    .key("time"s).value(item->spending.tripTime / 60)
                    .endDict();
        }
        json::Node x = builder.endArray().build();

        return json::Builder{}
        .startDict()
        .key("request_id"s).value(requestId)
        .key("total_time"s).value(totalTime / 60)
        .key("items"s).value(x.asArray())
        .endDict().build();
    } else {
        return makeNotFoundAnswer(requestId);
    }
}

RenderSettings JsonReader::parseRenderSettings(const json::Node& node) {
    const json::Dict& settingsDict = node.asDict();
    RenderSettings renderSettings;
    renderSettings.width = settingsDict.at("width"s).asDouble();
    renderSettings.height = settingsDict.at("height"s).asDouble();

    renderSettings.padding = settingsDict.at("padding"s).asDouble();
    renderSettings.lineWidth = settingsDict.at("line_width"s).asDouble();
    renderSettings.stopRadius = settingsDict.at("stop_radius"s).asDouble();

    renderSettings.busLabelFontSize = settingsDict.at("bus_label_font_size"s).asInt();
    json::Array rawBusLabelOffset = settingsDict.at("bus_label_offset"s).asArray();
    renderSettings.busLabelOffset.x = rawBusLabelOffset[0].asDouble();
    renderSettings.busLabelOffset.y = rawBusLabelOffset[1].asDouble();


    renderSettings.stopLabelFontSize = settingsDict.at("stop_label_font_size"s).asInt();
    json::Array rawStopLabelOffset = settingsDict.at("stop_label_offset"s).asArray();
    renderSettings.stopLabelOffset.x = rawStopLabelOffset[0].asDouble();
    renderSettings.stopLabelOffset.y = rawStopLabelOffset[1].asDouble();


    renderSettings.underlayerColor = nodeToColor(settingsDict.at("underlayer_color"s));
    renderSettings.underlayerWidth = settingsDict.at("underlayer_width"s).asDouble();

    for (const json::Node& n: settingsDict.at("color_palette"s).asArray()) {
        renderSettings.colorPalette.push_back(nodeToColor(n));
    }
    return renderSettings;
}

JsonReader::RoutingSettings JsonReader::parseRoutingSettings(const json::Node& node) {
    const json::Dict& settingsDict = node.asDict();
    RoutingSettings routingSettings;
    routingSettings.busWaitTime = settingsDict.at("bus_wait_time"s).asInt();
    routingSettings.busVelocity = settingsDict.at("bus_velocity"s).asDouble();
    return routingSettings;
}

std::vector<JsonReader::Request> JsonReader::parseStatRequests(const json::Node& node) {
    std::vector<Request> result;
    for (const json::Node& n: node.asArray()) {
        Request r;
        r.id = n.asDict().at("id"s).asInt();
        if (n.asDict().at("type"s).asString() == "Stop") {
            r.type = REQUEST_TYPE::STOP;
            r.name = n.asDict().at("name"s).asString();
        } else if (n.asDict().at("type"s).asString() == "Bus") {
            r.type = REQUEST_TYPE::BUS;
            r.name = n.asDict().at("name"s).asString();
        } else if (n.asDict().at("type"s).asString() == "Map") {
            r.type = REQUEST_TYPE::MAP;
        }else if (n.asDict().at("type"s).asString() == "Route") {
            r.type = REQUEST_TYPE::ROUTE;
            r.from = n.asDict().at("from"s).asString();
            r.to = n.asDict().at("to"s).asString();
        } else {
            throw json::ParsingError("Stat request type parsing error"s);
        }
        result.push_back(r);
    }
    return result;
}

void JsonReader::fillDataBase(transport::Catalogue& catalogue,
                              const std::vector<const json::Node *> &stopNodes,
                              const std::vector<const json::Node *> &routeNodes) {
    using namespace json;
    struct Distance {
        std::string from;
        std::string to;
        int meters;
    };
    std::vector<Distance> distances;

    // Stops
    for (const Node* node: stopNodes) {
        std::string name = node->asDict().at("name"s).asString();
        double latitude  = node->asDict().at("latitude"s).asDouble();
        double longitude  = node->asDict().at("longitude"s).asDouble();
        catalogue.addStop(name, {latitude, longitude});
        for(const auto& d: node->asDict().at("road_distances"s).asDict()) {
            distances.push_back({name,d.first,d.second.asInt()});
        }
    }

    // Distances
    for (const Distance& distance: distances) {
        catalogue.setDistance(distance.from, distance.to, distance.meters);
    }

    // Routes
    for (const Node* n: routeNodes) {
        std::string name = n->asDict().at("name"s).asString();
        bool isCycled = n->asDict().at("is_roundtrip"s).asBool();
        std::vector<transport::Stop*> stops;
        for (const Node& n: n->asDict().at("stops"s).asArray()) {
            if (auto stop = catalogue.stop(n.asString()); stop) {
                stops.push_back(*stop);
            } else {
                throw std::invalid_argument("Invalid stop: "s + n.asString());
            }
        }
        catalogue.addRoute(name, std::move(stops), isCycled);
    }
}

json::Node JsonReader::prepareAnswers(std::vector<Request>& requests,
                                      transport::Catalogue& catalogue,
                                      MapRenderer& renderer,
                                      transport::RouteFinder& finder) {
    // Отказываемся от использования билдера, поскольку это существенно ускоряет код
    json::Array answers(requests.size());
    for (int i = 0; i < static_cast<int>(answers.size()); ++i) {
        auto request = requests[i];
        switch (request.type) {
        case REQUEST_TYPE::STOP: {
            answers[i] = makeStopAnswer(request.id, request.name, catalogue);
        } break;
        case REQUEST_TYPE::BUS: {
            answers[i] = makeRouteAnswer(request.id, request.name, catalogue);
        } break;
        case REQUEST_TYPE::MAP: {
            answers[i] = makeMapAnswer(request.id, renderer);
        } break;
        case REQUEST_TYPE::ROUTE: {
            answers[i] = makePathAnswer(request.id, request.from, request.to, finder);
        } break;
        default:
            throw std::exception();
        }
    }
    return answers;
}
