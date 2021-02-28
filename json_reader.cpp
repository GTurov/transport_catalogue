#include "json_reader.h"

#include "json_builder.h"

#include <vector>
#include <optional>

using namespace std::literals;

json::Node makeStopAnswer(int request_id, const transport::Stop::Info& data) {
    json::Builder builder;
    builder.StartArray();
    for (transport::Route* r: data.routes) {
        builder.Value(r->name());
    }
    json::Node x = builder.EndArray().Build();
    return json::Builder{}
            .StartDict()
            .Key("request_id"s).Value(request_id)
            .Key("buses"s).Value(x.AsArray())
            .EndDict().Build();
}

json::Node makeRouteAnswer(int request_id, const transport::Route::Info& data) {
    return json::Builder{}
    .StartDict()
    .Key("request_id"s).Value(request_id)
    .Key("route_length"s).Value(data.length)
    .Key("stop_count"s).Value(data.stopCount)
    .Key("unique_stop_count"s).Value(data.uniqueStopCount)
    .Key("curvature"s).Value(data.curvature)
    .EndDict()
    .Build()
    ;

}

svg::Color nodeToColor(const json::Node& n) {
    if (n.IsString()) {
        return svg::Color(n.AsString());
    }
    if (n.IsArray()) {
        const json::Array& color = n.AsArray();
        if (color.size() == 3) {
            return svg::Color(
                        svg::Rgb(color[0].AsInt(),color[1].AsInt(),color[2].AsInt()));
        }

        if (color.size() == 4) {
            return svg::Color(
                        svg::Rgba(color[0].AsInt(),color[1].AsInt(),color[2].AsInt(),
                    color[3].AsDouble()));
        }
    }
    throw std::exception();
}

void JsonReader::processQueries(std::istream& in, std::ostream& out) {
    using namespace json;

    const Document raw_requests = json::Load(in);

    const Node& base_requests = raw_requests.GetRoot().AsDict().at("base_requests");

    std::vector<const Node*> stop_nodes;
    std::vector<const Node*> bus_nodes;

    for (const Node& n: base_requests.AsArray()) {
        if (n.AsDict().at("type"s).AsString() == "Stop"s) {
            stop_nodes.push_back(&n);
            continue;
        }
        if (n.AsDict().at("type"s).AsString() == "Bus"s) {
            bus_nodes.push_back(&n);
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
    for (const Node* n: stop_nodes) {
        std::string name = n->AsDict().at("name"s).AsString();
        double latitude  = n->AsDict().at("latitude"s).AsDouble();
        double longitude  = n->AsDict().at("longitude"s).AsDouble();
        catalogue_.addStop(name, {latitude, longitude});
        for(const auto& d: n->AsDict().at("road_distances"s).AsDict()) {
            distances.push_back({name,d.first,d.second.AsInt()});
        }
    }

    // Distances
    for (const distance& d: distances) {
        catalogue_.setDistance(d.from, d.to, d.meters);
    }


    // Routes
    for (const Node* n: bus_nodes) {
        std::string name = n->AsDict().at("name"s).AsString();
        bool isCycled = n->AsDict().at("is_roundtrip"s).AsBool();
        std::vector<transport::Stop*> stops;
        for (const Node& n: n->AsDict().at("stops"s).AsArray()) {
            if (auto stop = catalogue_.stop(n.AsString()); stop) {
                stops.push_back(*stop);
            } else {
                throw std::invalid_argument("Invalid stop: "s + n.AsString());
            }
        }
        catalogue_.addRoute(name, std::move(stops), isCycled);
    }

    // Render settings
    if (raw_requests.GetRoot().AsDict().find("render_settings") != raw_requests.GetRoot().AsDict().end()) {
        const Dict& render_settings = raw_requests.GetRoot().AsDict().at("render_settings"s).AsDict();
        rs_.width = render_settings.at("width"s).AsDouble();
        rs_.height = render_settings.at("height"s).AsDouble();

        rs_.padding = render_settings.at("padding"s).AsDouble();
        rs_.lineWidth = render_settings.at("line_width"s).AsDouble();
        rs_.stopRadius = render_settings.at("stop_radius"s).AsDouble();

        rs_.busLabelFontSize = render_settings.at("bus_label_font_size"s).AsInt();
        Array raw_bus_label_offset = render_settings.at("bus_label_offset"s).AsArray();
        rs_.busLabelOffset.x = raw_bus_label_offset[0].AsDouble();
        rs_.busLabelOffset.y = raw_bus_label_offset[1].AsDouble();


        rs_.stopLabelFontSize = render_settings.at("stop_label_font_size"s).AsInt();
        Array raw_stop_label_offset = render_settings.at("stop_label_offset"s).AsArray();
        rs_.stopLabelOffset.x = raw_stop_label_offset[0].AsDouble();
        rs_.stopLabelOffset.y = raw_stop_label_offset[1].AsDouble();


        rs_.underlayerColor = nodeToColor(render_settings.at("underlayer_color"s));
        rs_.underlayerWidth = render_settings.at("underlayer_width"s).AsDouble();

        for (const Node& n: render_settings.at("color_palette"s).AsArray()) {
            rs_.colorPalette.push_back(nodeToColor(n));
        }
    }
    MapRenderer renderer(catalogue_, rs_);


    // Requests
    const Node& stat_requests = raw_requests.GetRoot().AsDict().at("stat_requests");

    std::vector<request> pure_requests;
    for (const Node& n: stat_requests.AsArray()) {
        request r;
        r.id = n.AsDict().at("id"s).AsInt();
        if (n.AsDict().at("type"s).AsString() == "Stop") {
            r.type = request_type::REQUEST_STOP;
            r.name = n.AsDict().at("name"s).AsString();
        } else if (n.AsDict().at("type"s).AsString() == "Bus") {
            r.type = request_type::REQUEST_BUS;
            r.name = n.AsDict().at("name"s).AsString();
        } else if (n.AsDict().at("type"s).AsString() == "Map") {
            r.type = request_type::REQUEST_MAP;
        } else {
            throw json::ParsingError("Stat request type parsing error"s);
        }
        pure_requests.push_back(r);
    }

    // Process requests
    json::Builder answers;
    answers.StartArray();
    for (const request& r: pure_requests) {
        switch (r.type) {
        case request_type::REQUEST_STOP: {
            if (auto stop_info = catalogue_.stopInfo(r.name); stop_info) {
                answers.Value(makeStopAnswer(r.id, *stop_info).AsDict());
            } else {
                answers.Value(
                json::Builder{}
                            .StartDict()
                            .Key("request_id"s).Value(r.id)
                            .Key("error_message"s).Value("not found"s)
                            .EndDict()
                            .Build().AsDict()
                            );
            }
        } break;
        case request_type::REQUEST_BUS: {
            if (auto route_info = catalogue_.routeInfo(r.name); route_info) {
                answers.Value(makeRouteAnswer(r.id, *route_info).AsDict());
            } else {
                answers.Value(
                json::Builder{}
                            .StartDict()
                            .Key("request_id"s).Value(r.id)
                            .Key("error_message"s).Value("not found"s)
                            .EndDict()
                            .Build().AsDict()
                            );
            }
        } break;
        case request_type::REQUEST_MAP: {
            answers.Value(
            json::Builder{}
                        .StartDict()
                        .Key("request_id"s).Value(r.id)
                        .Key("map"s).Value(renderer.render())
                        .EndDict()
                        .Build().AsDict()
                        );
        } break;
        default:
            throw std::exception();
        }
    }
    answers.EndArray();
    out << answers.Build();
}
