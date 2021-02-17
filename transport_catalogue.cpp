#include "transport_catalogue.h"

#include "domain.h"

//#include <iomanip>
//#include <iostream>
#include <stdexcept>
#include <numeric>


namespace transport {

size_t detail::RouteNumberHasher::operator() (const std::string_view text) const {
    int mult = 1;
    return std::accumulate(text.begin(), text.end(), int{0},[&mult](int sum, char c){mult *=100; return sum + (c-'A')*mult;});
}

size_t detail::StopNameHasher::operator() (const std::string_view text) const {
    int mult = 1;
    auto hash_end = text.begin();
    if (text.size() > 4) {
        std::advance(hash_end,4);
    } else {
        hash_end = text.end();
    }
    size_t sum = std::accumulate(text.begin(), hash_end, int{0},[&mult](int sum, char c){mult *=100; return sum + (c-'A')*mult;});
    return sum*text.size();
}

size_t detail::StopPairHasher::operator() (const std::pair<Stop*,Stop*> stops) const {
    // Будем считать, что для x64 вероятность совпадения "хвостов" адресов невелика :)
    return ((((size_t)stops.first<<16)&0xFFFF0000) | ((size_t)stops.second&0x0000FFFF));
}

Catalogue::~Catalogue() {
    name_to_bus_.clear();
    name_to_stop_.clear();
    stop_to_buses_.clear();
    stops_to_distances_.clear();
    while (!routes_.empty()) {
        delete routes_.back();
        routes_.pop_back();
    }
    while (!stops_.empty()) {
        delete stops_.back();
        stops_.pop_back();
    }
}

void Catalogue::addStop(Stop* stop) {
    stops_.push_back(stop);
    name_to_stop_[stop->name()] = stop;
    if (stop_to_buses_.find(stop) == stop_to_buses_.end()) {
        stop_to_buses_[stop] = {};
    }
}

void Catalogue::addStop(const std::string_view name, const geo::Coordinates &place) {
    auto* stop = new Stop(name, place);
    //std::cout<<*stop<<std::endl; // debug
    addStop(stop);
}

void Catalogue::addRoute(Route* route) {
    routes_.push_back(route);
    name_to_bus_[route->name()] = route;
    int length_ = 0;
    for (int i = 0; i < (int)route->stops().size()-1; ++i) {
        length_ += distanceBetween(route->stops()[i],route->stops()[i+1]);
    }
    if (!route->isCycled()) {
        length_ += distanceBetween(route->stops()[route->stops().size()-1],
                route->stops()[route->stops().size()-1]);
        for (int i = (int)route->stops().size()-1; i > 0; --i) {
            length_ += distanceBetween(route->stops()[i],route->stops()[i-1]);
        }
    }
    route->setLength(length_);
    for (auto * stop: route->stops()) {
        stop_to_buses_[stop].insert(route);
    }
}
void Catalogue::addRoute(const std::string_view name, const std::vector<Stop *>& stops, bool cycled) {
    auto* route = new Route(name, stops, cycled);
    //std::cout<<*route<<std::endl; // debug
    addRoute(route);
}

void Catalogue::setDistance(Stop* first, Stop* second, int meters) {
    if ((first!= nullptr) && (second != nullptr)) {
        stops_to_distances_[{first, second}] = meters;
    } else {
        throw std::invalid_argument("Invalid stop"s);
    }
}

void Catalogue::setDistance(const std::string_view first, const std::string_view second, int meters) {
    if ((name_to_stop_.find(first) == name_to_stop_.end()) || (name_to_stop_.find(second) == name_to_stop_.end())) {
        throw std::invalid_argument("Invalid stop"s);
    }
    setDistance(name_to_stop_[first], name_to_stop_[second], meters);
}

std::optional<Route*> Catalogue::route(const std::string_view name) const {
    if (name_to_bus_.find(name) == name_to_bus_.end()) {
        return std::nullopt;
    };
    return name_to_bus_.at(name);
}
std::optional<Stop*> Catalogue::stop(const std::string_view name) const {
    if (name_to_stop_.find(name) == name_to_stop_.end()) {
        return std::nullopt;
    };
    return name_to_stop_.at(name);
}

const RouteSet Catalogue::routesViaStop(const std::string_view name) const {
    if (stop_to_buses_.find(name_to_stop_.at(name)) == stop_to_buses_.end()) {
        return {};
    };
    return stop_to_buses_.at(name_to_stop_.at(name));
}

std::optional<const Route::Info> Catalogue::routeInfo(const std::string_view name) const {
    if (name_to_bus_.find(name) == name_to_bus_.end()) {
        return std::nullopt;
    };
    Route::Info result;
    result.name = name;
    Route * route = name_to_bus_.at(name);
    result.stopCount = route->stopsCount();
    result.uniqueStopCount = route->uniqueStopCount();
    result.length = route->length();
    result.curvature = route->curvature();
    return result;
}

std::optional<const Stop::Info> Catalogue::stopInfo(const std::string_view name) const {
    if (name_to_stop_.find(name) == name_to_stop_.end()) {
        return std::nullopt;
    }
    Stop::Info result;
    result.name = name;
    result.routes = stop_to_buses_.at(name_to_stop_.at(name));
    return result;
}

int Catalogue::distanceBetween(Stop* first, Stop* second) const {
    if ((first!= nullptr) && (second != nullptr)) {
        if (stops_to_distances_.find({first, second}) != stops_to_distances_.end()) {
            return stops_to_distances_.at({first, second});
        } else if (stops_to_distances_.find({second, first}) != stops_to_distances_.end()) {
            return stops_to_distances_.at({second, first});
        } else if (first == second) {
            return 0;
        } else {
            throw std::invalid_argument("No distance data"s);
        }
    } else {
        throw std::invalid_argument("Invalid stop"s);
    }
}
int Catalogue::distanceBetween(const std::string_view first, const std::string_view second) const {
    if ((name_to_stop_.find(first) == name_to_stop_.end()) || (name_to_stop_.find(second) == name_to_stop_.end())) {
        throw std::invalid_argument("Invalid stop"s);
    }
    return distanceBetween(name_to_stop_.at(first), name_to_stop_.at(second));
}

}
