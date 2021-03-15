#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <iostream> // debug
#include <unordered_map>
#include <vector>

namespace transport {

struct Trip {
    transport::Stop* from = nullptr;
    transport::Stop* to = nullptr;
    int stopCount = 0;
    double time = 0;
};

class RouteFinder {
    using route = graph::Router<double>::RouteInfo;
public:
    RouteFinder(Catalogue& catalogue, int bus_wait_time , double bus_velocity)
        : catalogue_(catalogue),
          bus_wait_time_(bus_wait_time),
          bus_velocity_(bus_velocity) {
        // Все остановки будут вершинами графа. Добавим их в словарь для быстрого поиска вершины по названию.
        for (auto* stop: catalogue_.allStops()) {
            graphVertexes_.push_back(stop);
            stopToGraphVertex_.insert({stop->name(),graphVertexes_.size()-1});
        }
        for (auto* route: catalogue_.allRoutes()) {

        }

    }
    std::optional<route> getRoute(std::string_view from, std::string_view to) {
        return std::nullopt;
    }

private:
    Catalogue& catalogue_;
    //graph::Router<double> router_;
    graph::DirectedWeightedGraph<double> graph_;
    std::unordered_map<std::string_view, int> stopToGraphVertex_;
    std::vector<transport::Stop*> graphVertexes_;
    std::vector<Trip> graphEdges_;
    int bus_wait_time_ = 0;
    double bus_velocity_ = 0;

};

} // namespace transport
