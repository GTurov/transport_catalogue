#include "transport_router.h"

#include <iostream>

namespace transport {

TripSpending operator+ (const TripSpending& lhs, const TripSpending& rhs) {
    return {lhs.stopCount + rhs.stopCount,
                lhs.waitTime + rhs.waitTime,
                lhs.tripTime + rhs.tripTime};
}

bool operator< (const TripSpending& lhs, const TripSpending& rhs) {
    return (lhs.waitTime + lhs.tripTime < rhs.waitTime + rhs.tripTime);
}

bool operator> (const TripSpending& lhs, const TripSpending& rhs) {
    return (lhs.waitTime + lhs.tripTime > rhs.waitTime + rhs.tripTime);
}

std::ostream& operator<<(std::ostream& out, const TripItem& item) {
    out << item.from->name() << " -> "s << item.to->name()
        << " by bus "s << item.bus->name() << " : "s << item.spending.stopCount << " stops, "s
        << item.spending.waitTime / 60 << " min wait time "s
        << item.spending.tripTime / 60 << " min trip time"s;
    return out;
}

DistanceFinder::DistanceFinder(const Catalogue& catalogue, const Route* route)
    : direct_distances_(route->stops().size()),
      reverse_distances_(route->stops().size())
{
    int directDistanceSum = 0;
    int reverseDistanceSum = 0;
    direct_distances_[0] = directDistanceSum;
    reverse_distances_[0] = reverseDistanceSum;
    for (int i = 1; i < (int)route->stops().size(); ++i) {
        directDistanceSum += catalogue.distanceBetween(route->stops()[i-1],route->stops()[i]);
        direct_distances_[i] = directDistanceSum;
        reverseDistanceSum += catalogue.distanceBetween(route->stops()[i],route->stops()[i-1]);
        reverse_distances_[i] = reverseDistanceSum;
    }
}

int DistanceFinder::distanceBetween(int fromStopIndex, int toStopIndex) {
    if (fromStopIndex < toStopIndex) {
        return direct_distances_[toStopIndex] - direct_distances_[fromStopIndex];
    } else {
        return -(reverse_distances_[toStopIndex] - reverse_distances_[fromStopIndex]);
    }
}

RouteFinder::RouteFinder(const Catalogue& catalogue, int bus_wait_time , double bus_velocity)
    : catalogue_(catalogue),
      bus_wait_time_(bus_wait_time*60),
      bus_velocity_(bus_velocity/3.6) {

    // Все остановки будут вершинами графа. Добавим их в словарь для быстрого поиска вершины по названию.
    auto allStops = catalogue_.allStops();

    // Создаём сам граф с нужным количеством вершин
    graph_ = std::make_unique<NavigationGraph>(allStops.size());

    // Добавляем вершины
    graph::VertexId vertexCount = 0;
    for (auto* stop: allStops) {
        stopToGraphVertex_.insert({stop,vertexCount++});
    }

    // Добавляем грани
    for (auto* route: catalogue_.allRoutes()) {
        DistanceFinder df(catalogue,route);
        const auto& stops = route->stops();
        for (int i = 0; i+1 < (int)stops.size(); ++i) {
            for (int j = i+1; j < (int)stops.size(); ++j) {
                addTripItem(stops[i], stops[j], route, {abs(i-j), static_cast<double>(bus_wait_time_), df.distanceBetween(i,j)/bus_velocity_});
                if (!route->isCycled()) {
                    addTripItem(stops[j], stops[i], route, {abs(i-j), static_cast<double>(bus_wait_time_), df.distanceBetween(j,i)/bus_velocity_});
                }
            }
        }
    }
    router_ = std::make_unique<graph::Router<GraphWeight>>(*graph_);
}

std::optional<std::vector<const TripItem *> > RouteFinder::findRoute(std::string_view from, std::string_view to) {
    auto stopFrom = catalogue_.stop(from);
    auto stopTo = catalogue_.stop(to);
    if (!stopFrom.has_value() || !stopTo.has_value()) {
        return std::nullopt;
    }

    std::vector<const TripItem*> result;
    if (stopFrom == stopTo) {
        return result;
    }

    graph::VertexId fromVertexId = stopToGraphVertex_.at(stopFrom.value());
    graph::VertexId toVertexId = stopToGraphVertex_.at(stopTo.value());
    auto route = router_->BuildRoute(fromVertexId, toVertexId);
    if (!route.has_value()) {
        return std::nullopt;
    }

    for (const auto& edge: route.value().edges) {
        result.push_back(&graphEdges_.at(edge));
    }
    return result;
}

void RouteFinder::addTripItem(const Stop* from, const Stop* to, const Route* route, TripSpending &&spending) {
    TripItem item{from, to, route, spending};
    int id = graph_->AddEdge(graph::Edge<GraphWeight>{stopToGraphVertex_[item.from],
                                                      stopToGraphVertex_[item.to], item.spending});
    graphEdges_.push_back(std::move(item));
    if (id != (int)graphEdges_.size()-1) {
        throw std::exception();
    }
}

} // namespace transport
