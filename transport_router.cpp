#include "transport_router.h"

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
        << " by bus " << item.bus->name() << " : " << item.spending.stopCount << " stops, "
        << item.spending.waitTime / 60 << " min wait time "
        << item.spending.tripTime / 60 << " min trip time";
    return out;
}

DistanceFinder::DistanceFinder(Catalogue& catalogue, const Route* route)
    : direct_distances_(route->stops().size(),0),
      reverse_distances_(route->stops().size(),0)
{
    int directDistanceSum = 0;
    int reverseDistanceSum = 0;
    for (int i = 1; i < (int)route->stops().size(); ++i) {
        directDistanceSum += catalogue.distanceBetween(route->stops()[i-1],route->stops()[i]);
        direct_distances_[i] = directDistanceSum;
        reverseDistanceSum += catalogue.distanceBetween(route->stops()[i],route->stops()[i-1]);
        reverse_distances_[i] = reverseDistanceSum;
        //std::cerr << i << " " << direct_distances_[i] << " " << reverse_distances_[i]<<std::endl;
    }
}

int DistanceFinder::distanceBetween(int fromStopIndex, int toStopIndex) {
    if (fromStopIndex < toStopIndex) {
        return direct_distances_[toStopIndex] - direct_distances_[fromStopIndex];
    } else {
        return -(reverse_distances_[toStopIndex] - reverse_distances_[fromStopIndex]);
    }
}

RouteFinder::RouteFinder(Catalogue& catalogue, int bus_wait_time , double bus_velocity)
    : catalogue_(catalogue),
      bus_wait_time_(bus_wait_time*60),
      bus_velocity_(bus_velocity/3.6) {

    // Все остановки будут вершинами графа. Добавим их в словарь для быстрого поиска вершины по названию.
    auto allStops = catalogue_.allStops();

    // Создаём сам граф с нужным количеством вершин
    graph_ = std::make_unique<NavigationGraph>(allStops.size());

    // Добавляем вершины и петли для тех остановок, через которые не ходят автобусы
    graph::VertexId vertexCount = 0;
    for (auto* stop: allStops) {
        //graphVertexes_.push_back(stop);
        stopToGraphVertex_.insert({stop,vertexCount++});
        //if (catalogue.routesViaStop(stop->name()).size() == 0) {
        //    addTripItem(stop, stop, nullptr, {0,0,0});
        //}
    }

    // Добавляем грани
    for (auto* route: catalogue_.allRoutes()) {
        DistanceFinder df(catalogue,route);
        const auto& stops = route->stops();
        //std::cerr<< "Route " << route->name() <<std::endl;
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

std::optional<std::vector<TripItem>> RouteFinder::findRoute(std::string_view from, std::string_view to) {
    //std::cerr<<"Find route from "s << from << " to "s << to << std::endl;
    auto stopFrom = catalogue_.stop(from);
    auto stopTo = catalogue_.stop(to);
    if (!stopFrom.has_value() || !stopTo.has_value()) {
        return std::nullopt;
    }

    std::vector<TripItem> result;
    if (stopFrom == stopTo) {
        return result;
    }

    graph::VertexId fromVertexId = stopToGraphVertex_.at(stopFrom.value());
    graph::VertexId toVertexId = stopToGraphVertex_.at(stopTo.value());
    auto route = router_->BuildRoute(fromVertexId, toVertexId);
    if (!route.has_value()) {
        //std::cerr << "Route not found\n";
        return std::nullopt;
    }

    //std::cerr << "Route found:" << (route.value().weight.waitTime + route.value().weight.tripTime) / 60 << " min trip time:\n";
    for (const auto& edge: route.value().edges) {
        //std::cerr << graphEdges_.at(edge) << std::endl;
        result.push_back(graphEdges_.at(edge));
    }
    //std::cerr << std::endl << std::endl;

    return result;
}

void RouteFinder::addTripItem(Stop* from, Stop* to, Route* route, TripSpending &&spending) {
    TripItem item{from, to, route, spending};
    //std::cerr<<"Edge "<<item << ", "s<<df.distanceBetween(i,j)<<" m"<<std::endl;
    int id = graph_->AddEdge(graph::Edge<GraphWeight>{stopToGraphVertex_[item.from],
                                                      stopToGraphVertex_[item.to], item.spending});
    graphEdges_.push_back(std::move(item));
    if (id != (int)graphEdges_.size()-1) {
        throw std::exception();
    }
}

} // namespace transport
