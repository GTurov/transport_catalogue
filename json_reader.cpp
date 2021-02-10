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

    // Requests
    const Node stat_requests = raw_requests.GetRoot().AsMap().at("stat_requests");
    //out<<"Stat:\n"s<<stat_requests.Content();

    std::vector<request> pure_requests;
    for (const Node& n: stat_requests.AsArray()) {
        request r;
        r.id = n.AsMap().at("id"s).AsInt();
        r.name = n.AsMap().at("name"s).AsString();
        if(n.AsMap().at("type"s).AsString() == "Stop") {
            r.type = request_type::REQUEST_STOP;
        } else if(n.AsMap().at("type"s).AsString() == "Bus") {
            r.type = request_type::REQUEST_BUS;
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
        default:
            throw std::exception();
        }
    }

    out << answers;


    //out<<requests.GetRoot().Content();
}
