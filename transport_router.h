#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <iostream> // debug
#include <unordered_map>
#include <vector>

namespace transport {

struct TripTime {
    double wait = 0;
    double ride = 0;
};

struct TripItem {
    transport::Stop* from = nullptr;
    transport::Stop* to = nullptr;
    int stopCount = 0;
    double time = 0;
};

inline std::ostream& operator<<(std::ostream& out, const TripItem& item) {
    out << item.from->name() << " -> "s << item.to->name()
        << ": " << item.stopCount << " stops, " << item.time << " sec trip time";
    return out;
}

// Вспомогательный класс для вычисления расстояния между всеми остановками
// за линейное время
class DistanceFinder {
public:
    DistanceFinder(Catalogue& catalogue, const Route* route);
    int distanceBetween(int fromStopIndex, int toStopIndex);
private:
    std::vector<int> direct_distances_;
    std::vector<int> reverse_distances_;

};

class RouteFinder {
    using route = graph::Router<double>::RouteInfo;

public:
    RouteFinder(Catalogue& catalogue, int bus_wait_time , double bus_velocity);
    std::optional<std::vector<TripItem>> findRoute(std::string_view from, std::string_view to);

private:
    std::optional<route> buildRoute(std::string_view from, std::string_view to);

private:
    Catalogue& catalogue_;
    //graph::Router<double> router_;
    graph::DirectedWeightedGraph<double> graph_;
    std::vector<transport::Stop*> graphVertexes_;
    std::unordered_map<std::string_view, int> stopToGraphVertex_;
    std::vector<TripItem> graphEdges_;
    int bus_wait_time_ = 0;
    double bus_velocity_ = 0;

};

} // namespace transport
