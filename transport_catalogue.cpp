#include "transport_catalogue.h"

#include <ostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& out, const bus_stop& stop) {
    out << "Stop "s << stop.name() <<": "s;
    out << std::setprecision(6);
    out << stop.place().lat << " "s << stop.place().lng;
    return out;
}

//bool operator<(const bus_stop& lhs, const bus_stop& rhs) {
//    return std::lexicographical_compare(lhs.name().begin(), lhs.name().end(),
//                                        rhs.name().begin(), rhs.name().end());
//}

bus_route::bus_route(std::string_view name, std::vector<bus_stop*> stops, bool cycled)
    :name_(name), stops_(stops), isCycled_(cycled) {
    for (int i = 0; i < (int)stops.size()-1; ++i) {
        directLength_ += ComputeDistance(stops[i]->place(),stops[i+1]->place());
    }
    if (!cycled) {
        directLength_ *= 2;
    }
    uniqueStopCount_ = std::unordered_set<bus_stop*>(stops.begin(), stops.end()).size();
}

std::ostream& operator<<(std::ostream& out, const bus_route& route) {
    out << "Bus "s << route.name() <<": "s;
    bool firstStop = true;
    for (const bus_stop* stop: route.stops()) {
        out << (firstStop?""s:" > "s)<< stop->name();
        firstStop = false;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const route_info& route) {
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

std::ostream& operator<<(std::ostream& out, const stop_info& stop) {
    out << "Stop "s << stop.name <<":"s;
    if (stop.routes.size() == 0) {
        out<< " no buses";
    } else {
        out<< " buses";
        for (const bus_route* route: stop.routes) {
            out << " "s << route->name();
        }
    }
    return out;
}

transport_catalogue::~transport_catalogue() {
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

void transport_catalogue::addStop(bus_stop* stop) {
    stops_.push_back(stop);
    name_to_stop_[stop->name()] = stop;
    if (stop_to_buses_.find(stop) == stop_to_buses_.end()) {
        stop_to_buses_[stop] = {};
    }
}

void transport_catalogue::addStop(std::string_view name, Coordinates place) {
    auto* stop = new bus_stop(name, place);
    //std::cout<<*stop<<std::endl;
    addStop(stop);
}

void transport_catalogue::addRoute(bus_route* route) {
    routes_.push_back(route);
    name_to_bus_[route->name()] = route;
    int length_ = 0;
    for (int i = 0; i < (int)route->stops().size()-1; ++i) {
        //std::cout<<i<<" "s<< route->stops()[i]->name() << " > "s << route->stops()[i+1]->name()<<std::endl;
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
        //std::cout<<stop->name()<<"|"s;
        stop_to_buses_[stop].insert(route);
    }
}
void transport_catalogue::addRoute(std::string_view name, std::vector<bus_stop*> stops, bool cycled) {
    auto* route = new bus_route(name, stops, cycled);
    //std::cout<<*route<<std::endl;
    addRoute(route);
}

void transport_catalogue::setDistance(bus_stop* first, bus_stop* second, int meters) {
    if ((first!= nullptr) && (second != nullptr)) {
        stops_to_distances_[{first, second}] = meters;
    } else {
        throw std::invalid_argument("Invalid stop"s);
    }
}

void transport_catalogue::setDistance(const std::string_view& first, const std::string_view& second, int meters) {
    if ((name_to_stop_.find(first) == name_to_stop_.end()) || (name_to_stop_.find(second) == name_to_stop_.end())) {
        throw std::invalid_argument("Invalid stop"s);
    }
    setDistance(name_to_stop_[first], name_to_stop_[second], meters);
}

route_info transport_catalogue::routeInfo(const std::string_view& name) const {
    route_info result;
    result.name = name;
    if (name_to_bus_.find(name) == name_to_bus_.end()) {
        return result;
    };
    bus_route * route = name_to_bus_.at(name);
    result.stopCount = route->stopsCount();
    result.uniqueStopCount = route->uniqueStopCount();
    result.length = route->length();
    result.curvature = route->curvature();
    return result;
}

stop_info transport_catalogue::stopInfo(const std::string_view& name) const {
    stop_info result;
    result.name = name;
    if (stop_to_buses_.find(name_to_stop_.at(name)) == stop_to_buses_.end()) {
        throw std::out_of_range("Stop not found");
    };
    result.routes = stop_to_buses_.at(name_to_stop_.at(name));
    return result;
}

int transport_catalogue::distanceBetween(bus_stop* first, bus_stop* second) const {
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
int transport_catalogue::distanceBetween(const std::string_view& first, const std::string_view& second) const {
    if ((name_to_stop_.find(first) == name_to_stop_.end()) || (name_to_stop_.find(second) == name_to_stop_.end())) {
        throw std::invalid_argument("Invalid stop"s);
    }
    return distanceBetween(name_to_stop_.at(first), name_to_stop_.at(second));
}

