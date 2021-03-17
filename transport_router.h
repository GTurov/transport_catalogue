#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <iostream> // debug
#include <memory>
#include <unordered_map>
#include <vector>

namespace transport {

struct TripSpending {
    TripSpending() = default;
    TripSpending(int stopCount, double waitTime, double tripTime)
        : stopCount(stopCount), waitTime(waitTime), tripTime(tripTime){}
    int stopCount = 0;
    double waitTime = 0;
    double tripTime = 0;
};

TripSpending operator+ (const TripSpending& lhs, const TripSpending& rhs);
bool operator< (const TripSpending& lhs, const TripSpending& rhs);
bool operator> (const TripSpending& lhs, const TripSpending& rhs);

struct TripItem {
    transport::Stop* from = nullptr;
    transport::Stop* to = nullptr;
    transport::Route* bus = nullptr;
    TripSpending spending = {};
};

std::ostream& operator<<(std::ostream& out, const TripItem& item);

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
    using GraphWeight = TripSpending;
    using NavigationGraph = graph::DirectedWeightedGraph<GraphWeight>;
public:
    RouteFinder(Catalogue& catalogue, int bus_wait_time , double bus_velocity);
    std::optional<std::vector<const TripItem*>> findRoute(std::string_view from, std::string_view to);

private:
    void addTripItem(Stop* from, Stop* to, Route* route, TripSpending&& spending);

private:
    Catalogue& catalogue_;
    std::unique_ptr<graph::Router<GraphWeight>> router_;
    std::unique_ptr<NavigationGraph> graph_;
    std::unordered_map<Stop*, graph::VertexId> stopToGraphVertex_;
    std::vector<TripItem> graphEdges_;
    int bus_wait_time_ = 0;
    double bus_velocity_ = 0;

};

} // namespace transport
