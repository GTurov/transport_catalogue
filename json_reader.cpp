#include "json_reader.h"

#include "json_builder.h"

#include <vector>
#include <optional>

using namespace std::literals;

json::Node makeStopAnswer(int requestId, const transport::Stop::Info& data) {
    json::Builder builder;
    builder.startArray();
    for (transport::Route* r: data.routes) {
        builder.value(r->name());
    }
    json::Node x = builder.endArray().build();
    return json::Builder{}
            .startDict()
            .key("request_id"s).value(requestId)
            .key("buses"s).value(x.asArray())
            .endDict().build();
}

json::Node makeRouteAnswer(int requestId, const transport::Route::Info& data) {
    return json::Builder{}
    .startDict()
    .key("request_id"s).value(requestId)
    .key("route_length"s).value(data.length)
    .key("stop_count"s).value(data.stopCount)
    .key("unique_stop_count"s).value(data.uniqueStopCount)
    .key("curvature"s).value(data.curvature)
    .endDict()
    .build()
    ;

}

json::Node makePathAnswer(int requestId, const std::vector<const transport::TripItem*>& data) {
    json::Builder builder;
    builder.startArray();
    double totalTime = 0;
    for (const auto& item: data) {
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
}

svg::Color nodeToColor(const json::Node& n) {
    if (n.isString()) {
        return svg::Color(n.asString());
    }
    if (n.isArray()) {
        const json::Array& color = n.asArray();
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

    struct distance {
        std::string from;
        std::string to;
        int meters;
    };
    std::vector<distance> distances;

    // Stops
    for (const Node* n: stopNodes) {
        std::string name = n->asDict().at("name"s).asString();
        double latitude  = n->asDict().at("latitude"s).asDouble();
        double longitude  = n->asDict().at("longitude"s).asDouble();
        catalogue_.addStop(name, {latitude, longitude});
        for(const auto& d: n->asDict().at("road_distances"s).asDict()) {
            distances.push_back({name,d.first,d.second.asInt()});
        }
    }

    // Distances
    for (const distance& d: distances) {
        catalogue_.setDistance(d.from, d.to, d.meters);
    }


    // Routes
    for (const Node* n: busNodes) {
        std::string name = n->asDict().at("name"s).asString();
        bool isCycled = n->asDict().at("is_roundtrip"s).asBool();
        std::vector<transport::Stop*> stops;
        for (const Node& n: n->asDict().at("stops"s).asArray()) {
            if (auto stop = catalogue_.stop(n.asString()); stop) {
                stops.push_back(*stop);
            } else {
                throw std::invalid_argument("Invalid stop: "s + n.asString());
            }
        }
        catalogue_.addRoute(name, std::move(stops), isCycled);
    }

    // Render settings
    if (rawRequests.getRoot().asDict().find("render_settings") != rawRequests.getRoot().asDict().end()) {
        const Dict& renderSettings = rawRequests.getRoot().asDict().at("render_settings"s).asDict();
        renderSettings_.width = renderSettings.at("width"s).asDouble();
        renderSettings_.height = renderSettings.at("height"s).asDouble();

        renderSettings_.padding = renderSettings.at("padding"s).asDouble();
        renderSettings_.lineWidth = renderSettings.at("line_width"s).asDouble();
        renderSettings_.stopRadius = renderSettings.at("stop_radius"s).asDouble();

        renderSettings_.busLabelFontSize = renderSettings.at("bus_label_font_size"s).asInt();
        Array rawBusLabelOffset = renderSettings.at("bus_label_offset"s).asArray();
        renderSettings_.busLabelOffset.x = rawBusLabelOffset[0].asDouble();
        renderSettings_.busLabelOffset.y = rawBusLabelOffset[1].asDouble();


        renderSettings_.stopLabelFontSize = renderSettings.at("stop_label_font_size"s).asInt();
        Array rawStopLabelOffset = renderSettings.at("stop_label_offset"s).asArray();
        renderSettings_.stopLabelOffset.x = rawStopLabelOffset[0].asDouble();
        renderSettings_.stopLabelOffset.y = rawStopLabelOffset[1].asDouble();


        renderSettings_.underlayerColor = nodeToColor(renderSettings.at("underlayer_color"s));
        renderSettings_.underlayerWidth = renderSettings.at("underlayer_width"s).asDouble();

        for (const Node& n: renderSettings.at("color_palette"s).asArray()) {
            renderSettings_.colorPalette.push_back(nodeToColor(n));
        }
    }
    MapRenderer renderer(catalogue_, renderSettings_);


    // Routing settings
    if (rawRequests.getRoot().asDict().find("routing_settings") != rawRequests.getRoot().asDict().end()) {
        const Dict& render_settings = rawRequests.getRoot().asDict().at("routing_settings"s).asDict();
        routingSettings_.busWaitTime = render_settings.at("bus_wait_time"s).asInt();
        routingSettings_.busVelocity = render_settings.at("bus_velocity"s).asDouble();
    }
    transport::RouteFinder navigator(catalogue_, routingSettings_.busWaitTime, routingSettings_.busVelocity);

    // Requests
    const Node& statRequests = rawRequests.getRoot().asDict().at("stat_requests");

    std::vector<Request> pureRequests;
    for (const Node& n: statRequests.asArray()) {
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
        pureRequests.push_back(r);
    }

    // Process requests
    // Отказываемся от использования билдера, поскольку это существенно ускоряет код
    json::Array answers(pureRequests.size());
    for (int i = 0; i < static_cast<int>(answers.size()); ++i) {
        auto r = pureRequests[i];
        switch (r.type) {
        case REQUEST_TYPE::STOP: {
            if (auto stopInfo = catalogue_.stopInfo(r.name); stopInfo) {
                answers[i] = makeStopAnswer(r.id, *stopInfo);
            } else {
                answers[i] =
                json::Builder{}
                            .startDict()
                            .key("request_id"s).value(r.id)
                            .key("error_message"s).value("not found"s)
                            .endDict()
                            .build();
            }
        } break;
        case REQUEST_TYPE::BUS: {
            if (auto routeInfo = catalogue_.routeInfo(r.name); routeInfo) {
                answers[i] = makeRouteAnswer(r.id, *routeInfo);
            } else {
                answers[i] =
                json::Builder{}
                            .startDict()
                            .key("request_id"s).value(r.id)
                            .key("error_message"s).value("not found"s)
                            .endDict()
                            .build();
            }
        } break;
        case REQUEST_TYPE::MAP: {
            answers[i] =
            json::Builder{}
                        .startDict()
                        .key("request_id"s).value(r.id)
                        .key("map"s).value(renderer.render())
                        .endDict()
                        .build();
        } break;
        case REQUEST_TYPE::ROUTE: {
            if (auto pathInfo = navigator.findRoute(r.from, r.to); pathInfo) {
                answers[i] = makePathAnswer(r.id, pathInfo.value());
            } else {
                answers[i] =
                json::Builder{}
                            .startDict()
                            .key("request_id"s).value(r.id)
                            .key("error_message"s).value("not found"s)
                            .endDict()
                            .build();
            }
        } break;
        default:
            throw std::exception();
        }
    }
    out << Node(answers);
}
