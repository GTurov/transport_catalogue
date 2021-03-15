#include "transport_router.h"

namespace transport {


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
      bus_wait_time_(bus_wait_time),
      bus_velocity_(bus_velocity/3.6) {
    // Все остановки будут вершинами графа. Добавим их в словарь для быстрого поиска вершины по названию.
    for (auto* stop: catalogue_.allStops()) {
        graphVertexes_.push_back(stop);
        stopToGraphVertex_.insert({stop->name(),graphVertexes_.size()-1});
    }

    for (auto* route: catalogue_.allRoutes()) {
        DistanceFinder df(catalogue,route);
        const auto& stops = route->stops();
        if (route->isCycled()) {

        } else {
            for (int i = 0; i < (int)stops.size(); ++i) {
                for (int j = 0; j < (int)stops.size(); ++j) {
                    if (i != j) {
                        TripItem item{stops[i], stops[j], abs(i-j), df.distanceBetween(i,j)/bus_velocity_};
                        std::cerr<<"Eblo "<<item << ", "s<<df.distanceBetween(i,j)<<" m"<<std::endl;
                    }
                }
            }
        }
    }

}

} // namespace transport
