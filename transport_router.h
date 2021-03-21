#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

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
    const transport::Stop* from = nullptr;
    const transport::Stop* to = nullptr;
    const transport::Route* bus = nullptr;
    TripSpending spending = {};
};

std::ostream& operator<<(std::ostream& out, const TripItem& item);

// Вспомогательный класс для вычисления расстояния между всеми остановками
// за линейное время
class DistanceFinder {
public:
    DistanceFinder(const Catalogue& catalogue, const Route* route);
    int distanceBetween(int fromStopIndex, int toStopIndex);
private:
    std::vector<int> directDistances_;
    std::vector<int> reverseDistances_;

};

class RouteFinder {
    using GraphWeight = TripSpending;
    using NavigationGraph = graph::DirectedWeightedGraph<GraphWeight>;
public:
    RouteFinder(const Catalogue& catalogue, int busWaitTime , double busVelocity);
    std::optional<std::vector<const TripItem*>> findRoute(std::string_view from, std::string_view to);

private:
    void addTripItem(const Stop* from, const Stop* to, const Route* route, TripSpending&& spending);

private:
    const Catalogue& catalogue_;
    std::unique_ptr<graph::Router<GraphWeight>> router_;
    std::unique_ptr<NavigationGraph> graph_;
    std::unordered_map<const Stop*, graph::VertexId> stopToGraphVertex_;
    std::vector<TripItem> graphEdges_;
    int busWaitTime_ = 0;
    double busVelocity_ = 0;

};

} // namespace transport
