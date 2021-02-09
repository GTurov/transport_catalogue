#include "json_reader.h"

#include "json.h"

#include <vector>

using namespace std::literals;

void json_reader::process_queries(std::istream& in, std::ostream& out) {
    using namespace json;
    const Document requests = json::Load(in);

    const Node base_requests = requests.GetRoot().AsMap().at("base_requests");
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
        throw std::exception();
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
        out<<*catalogue_.route(name)<<std::endl;
    }


    const Node stat_requests = requests.GetRoot().AsMap().at("stat_requests");
    out<<"Stat:\n"s<<stat_requests.Content();

    //out<<requests.GetRoot().Content();
}
