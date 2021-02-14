#include "json_reader.h"

#include <vector>

using namespace std::literals;

json::Node makeStopAnswer(int request_id, transport::Stop::Info data) {
    json::Array routes;
    for (transport::Route* r: data.routes) {
        routes.push_back(json::Node(r->name()));
    }
    return json::Node(json::Dict{{"request_id"s,request_id},{"buses",std::move(routes)}});
}

json::Node makeRouteAnswer(int request_id, transport::Route::Info data) {
    return json::Node(json::Dict{{"request_id"s,request_id},{"route_length",data.length},
                                 {"stop_count"s, data.stopCount},{"unique_stop_count"s,data.uniqueStopCount},
                                 {"curvature"s,data.curvature}});
}

svg::Color nodeToColor(const json::Node& n) {
    if (n.IsString()) {
        //std::cout<<"String"s<<n.AsString()<<std::endl;
        return svg::Color(n.AsString());
    }
    if (n.IsArray()) {
        //std::cout<<"Array "s;
        if (n.AsArray().size() == 3) {
            uint8_t red = n.AsArray()[0].AsInt();
            uint8_t green = n.AsArray()[1].AsInt();
            uint8_t blue = n.AsArray()[2].AsInt();
//            std::cout<<"rgb "s<<n.AsArray()[0].AsInt()<<" "s
//                    <<n.AsArray()[1].AsInt()<<" "s
//                   <<n.AsArray()[2].AsInt()<<" "s
//                  <<std::endl;
            return svg::Color(svg::Rgb(red,green,blue));
        }

        if (n.AsArray().size() == 4) {
            uint8_t red = n.AsArray()[0].AsInt();
            uint8_t green = n.AsArray()[1].AsInt();
            uint8_t blue = n.AsArray()[2].AsInt();
            double opacity = n.AsArray()[3].AsDouble();
//            std::cout<<"rgba "s<<n.AsArray()[0].AsInt()<<" "s
//                    <<n.AsArray()[1].AsInt()<<" "s
//                   <<n.AsArray()[2].AsInt()<<" "s
//                     <<n.AsArray()[3].AsDouble()<<" "s
//                  <<std::endl;
            return svg::Color(svg::Rgba(red,green,blue,opacity));
        }
    }
    throw std::exception();
}

void json_reader::process_queries(std::istream& in, std::ostream& out) {
    using namespace json;
    const Document raw_requests = json::Load(in);

    const Node base_requests = raw_requests.GetRoot().AsMap().at("base_requests");
    //out<<"Base:\n"s<<base_requests.Content()<<std::endl;

    std::vector<const Node*> stop_nodes;
    std::vector<const Node*> bus_nodes;

    for (const Node& n: base_requests.AsArray()) {
        if (n.AsMap().at("type"s).AsString() == "Stop"s) {
            //out<<"stop "<<n.AsMap().at("name"s).AsString()<<std::endl;
            stop_nodes.push_back(&n);
            continue;
        }
        if (n.AsMap().at("type"s).AsString() == "Bus"s) {
            //out<<"bus "<<n.AsMap().at("name"s).AsString()<<std::endl;
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
        std::string name = n->AsMap().at("name"s).AsString();
        double latitude  = n->AsMap().at("latitude"s).AsDouble();
        double longitude  = n->AsMap().at("longitude"s).AsDouble();
        catalogue_.addStop(name, {latitude, longitude});
        //out<<"stop added("<<n->AsMap().at("road_distances"s).AsMap().size()<<")\n"s;
        for(const std::pair<std::string, Node>& d: n->AsMap().at("road_distances"s).AsMap()) {
            distances.push_back({name,d.first,d.second.AsInt()});
        }
        //out<<*catalogue_.stop(name)<<std::endl;
    }

    // Distances
    for (const distance& d: distances) {
        //out<<d.from<<" -> "s <<d.to<<" "<<d.meters<<" meters"<<std::endl;
        catalogue_.setDistance(d.from, d.to, d.meters);
    }


    // Routes
    for (const Node* n: bus_nodes) {
        std::string name = n->AsMap().at("name"s).AsString();
        bool isCycled = n->AsMap().at("is_roundtrip"s).AsBool();
        std::vector<transport::Stop*> stops;
        for (const Node& n: n->AsMap().at("stops"s).AsArray()) {
            stops.push_back(catalogue_.stop(n.AsString()));
        }
        catalogue_.addRoute(name, std::move(stops), isCycled);
        //out<<*catalogue_.route(name)<<std::endl;
    }

    // Render settings
    const Dict& render_settings = raw_requests.GetRoot().AsMap().at("render_settings").AsMap();
    rs_.width = render_settings.at("width"s).AsDouble();
    //std::cerr<<rs.width<<std::endl;
    rs_.height = render_settings.at("height"s).AsDouble();
    //std::cerr<<rs.height<<std::endl;

    rs_.padding = render_settings.at("padding"s).AsDouble();
    //std::cerr<<rs.padding<<std::endl;
    rs_.line_width = render_settings.at("line_width"s).AsDouble();
    //std::cerr<<rs.line_width<<std::endl;
    rs_.stop_radius = render_settings.at("stop_radius"s).AsDouble();
    //std::cerr<<rs.stop_radius<<std::endl;

    rs_.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsDouble();
    //std::cerr<<rs.bus_label_font_size<<std::endl;
    Array raw_bus_label_offset = render_settings.at("bus_label_offset"s).AsArray();
    rs_.bus_label_offset.x = raw_bus_label_offset[0].AsDouble();
    rs_.bus_label_offset.y = raw_bus_label_offset[1].AsDouble();
    //std::cout << rs.bus_label_offset[0] << " "s << rs.bus_label_offset[1]<<std::endl;


    rs_.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsDouble();
    //std::cerr<<rs.stop_label_font_size<<std::endl;
    Array raw_stop_label_offset = render_settings.at("stop_label_offset"s).AsArray();
    rs_.stop_label_offset.x = raw_stop_label_offset[0].AsDouble();
    rs_.stop_label_offset.y = raw_stop_label_offset[1].AsDouble();
    //std::cout << rs.stop_label_offset[0] << " "s << rs.stop_label_offset[1]<<std::endl;


    rs_.underlayer_color = nodeToColor(render_settings.at("underlayer_color"s));
    //std::cerr<<rs.underlayer_color<<std::endl;
    rs_.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();
    //std::cerr<<rs.underlayer_width<<std::endl;

    for (const Node& n: render_settings.at("color_palette"s).AsArray()) {
        rs_.color_palette.push_back(nodeToColor(n));
        //std::cerr<<nodeToColor(n)<<std::endl;
    }

    // Requests
    const Node stat_requests = raw_requests.GetRoot().AsMap().at("stat_requests");
    //out<<"Stat:\n"s<<stat_requests.Content();

    std::vector<request> pure_requests;
    for (const Node& n: stat_requests.AsArray()) {
        request r;
        r.id = n.AsMap().at("id"s).AsInt();
        if(n.AsMap().at("type"s).AsString() == "Stop") {
            r.type = request_type::REQUEST_STOP;
            r.name = n.AsMap().at("name"s).AsString();
        } else if(n.AsMap().at("type"s).AsString() == "Bus") {
            r.type = request_type::REQUEST_BUS;
            r.name = n.AsMap().at("name"s).AsString();
        } else if(n.AsMap().at("type"s).AsString() == "Map") {
            r.type = request_type::REQUEST_MAP;
        } else {
            throw json::ParsingError("Stat request type parsing error"s);
        }
        pure_requests.push_back(r);
        //throw json::ParsingError("Stat request parsing error"s);
    }

    // Process requests
    Array answers;
    for (const request& r: pure_requests) {
        switch (r.type) {
        case request_type::REQUEST_STOP: {
            try {
                transport::Stop::Info stop_info = catalogue_.stopInfo(r.name);
                answers.push_back(makeStopAnswer(r.id, stop_info));

            }  catch (std::exception& e) {
                answers.push_back(Node(Dict{{"request_id"s,r.id},{"error_message"s, "not found"s}}));
            }

        } break;
        case request_type::REQUEST_BUS: {
            //try {
                transport::Route::Info route_info = catalogue_.routeInfo(r.name);
                if (route_info.length != 0) {
                answers.push_back(makeRouteAnswer(r.id, route_info));
                } else {
            //}  catch (std::exception& e) {
                answers.push_back(Node(Dict{{"request_id"s,r.id},{"error_message"s, "not found"s}}));
                }
            //}
        } break;
        case request_type::REQUEST_MAP: {
            map_renderer renderer(catalogue_, rs_);
            answers.push_back(Node(Dict{{"request_id"s,r.id},{"map",renderer.render()}}));
        } break;
        default:
            throw std::exception();
        }
    }

    out << answers;


    //out<<requests.GetRoot().Content();
}
