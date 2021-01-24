#include "transport_catalogue.h"

#include <ostream>
#include <iomanip>
#include <stdexcept>

#include <iostream>

namespace transport {

std::ostream& operator<<(std::ostream& out, const Stop& stop) {
    out << "Stop "s << stop.name() <<": "s;
    out << std::setprecision(6);
    out << stop.place().lat << " "s << stop.place().lng;
    return out;
}

Route::Route(std::string_view name, std::vector<Stop*> stops, bool cycled)
    :name_(name), stops_(stops), isCycled_(cycled) {
    for (int i = 0; i < (int)stops.size()-1; ++i) {
        directLength_ += ComputeDistance(stops[i]->place(),stops[i+1]->place());
    }
    if (!cycled) {
        directLength_ *= 2;
    }
    uniqueStopCount_ = std::unordered_set<Stop*>(stops.begin(), stops.end()).size();
}

std::ostream& operator<<(std::ostream& out, const Route& route) {
    out << "Bus "s << route.name() <<": "s;
    bool firstStop = true;
    for (const Stop* stop: route.stops()) {
        out << (firstStop?""s:" > "s)<< stop->name();
        firstStop = false;
    }
    return out;
}

bool detail::RouteComparator::operator() (const Route* lhs, const Route* rhs) const {
    return std::lexicographical_compare(
                lhs->name().begin(), lhs->name().end(),
                rhs->name().begin(), rhs->name().end());
}

size_t detail::RouteNumberHasher::operator() (const std::string_view& text) const {
    size_t res = 0;
    for (int i = 0; i < (int)text.size(); ++i) {
        res += text[i]-'A';
        res *= 100;
    }
    return res;
}

size_t detail::StopNameHasher::operator() (const std::string_view& text) const {
    size_t res = 0;
    for (int i = 0; i < (text.size() > 4?4:(int)text.size()); ++i) {
        res += text[i]-'A';
        res *= 100;
    }
    res *= text.size();
    return res;
}

size_t detail::StopPairHasher::operator() (const std::pair<Stop*,Stop*>& stops) const {
    // Будем считать, что для x64 вероятность совпадения "хвостов" адресов невелика :)
    return ((((size_t)stops.first<<16)&0xFFFF0000) | ((size_t)stops.second&0x0000FFFF));
}

std::ostream& operator<<(std::ostream& out, const Route::Info& route) {
    out << "Bus "s << route.name <<": "s;
    if (route.stopCount != 0) {
        out << route.stopCount << " stops on route, "s;
        out << route.uniqueStopCount << " unique stops, "s;
        out << std::setprecision(6);
        out << route.length << " route length, "s;
        out << route.curvature << " curvature"s;
    } else {
        out << "not found"s;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Stop::Info& stop) {
    out << "Stop "s << stop.name <<":"s;
    if (stop.routes.size() == 0) {
        out<< " no buses";
    } else {
        out<< " buses";
        for (const Route* route: stop.routes) {
            out << " "s << route->name();
        }
    }
    return out;
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

void Catalogue::addStop(std::string_view name, Coordinates place) {
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
void Catalogue::addRoute(std::string_view name, std::vector<Stop*> stops, bool cycled) {
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

void Catalogue::setDistance(const std::string_view& first, const std::string_view& second, int meters) {
    if ((name_to_stop_.find(first) == name_to_stop_.end()) || (name_to_stop_.find(second) == name_to_stop_.end())) {
        throw std::invalid_argument("Invalid stop"s);
    }
    setDistance(name_to_stop_[first], name_to_stop_[second], meters);
}

Route::Info Catalogue::routeInfo(const std::string_view& name) const {
    Route::Info result;
    result.name = name;
    if (name_to_bus_.find(name) == name_to_bus_.end()) {
        return result;
    };
    Route * route = name_to_bus_.at(name);
    result.stopCount = route->stopsCount();
    result.uniqueStopCount = route->uniqueStopCount();
    result.length = route->length();
    result.curvature = route->curvature();
    return result;
}

Stop::Info Catalogue::stopInfo(const std::string_view& name) const {
    Stop::Info result;
    result.name = name;
    if (stop_to_buses_.find(name_to_stop_.at(name)) == stop_to_buses_.end()) {
        throw std::out_of_range("Stop not found");
    };
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
int Catalogue::distanceBetween(const std::string_view& first, const std::string_view& second) const {
    if ((name_to_stop_.find(first) == name_to_stop_.end()) || (name_to_stop_.find(second) == name_to_stop_.end())) {
        throw std::invalid_argument("Invalid stop"s);
    }
    return distanceBetween(name_to_stop_.at(first), name_to_stop_.at(second));
}

}
