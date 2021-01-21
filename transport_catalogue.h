#pragma once

#include "geo.h"

#include <deque>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <iostream> //debug

using namespace std::literals;

class bus_stop {
public:
    bus_stop(std::string_view name, Coordinates place)
        :name_(name), place_(place)  {}
    const Coordinates place() const {
        return place_;
    }
    const std::string& name() const {
        return name_;
    }
private:
    std::string name_;
    Coordinates place_;
};

std::ostream& operator<<(std::ostream& out, const bus_stop& stop);

bool operator<(const bus_stop& lhs, const bus_stop& rhs);

class bus_route {
public:
    bus_route(std::string_view name, std::vector<bus_stop*> stops, bool cycled = false)
        :name_(name), stops_(stops), isCycled_(cycled) {
        //directLength_ = 0;
        for (int i = 0; i < (int)stops.size()-1; ++i) {
            directLength_ += ComputeDistance(stops[i]->place(),stops[i+1]->place());
        }
        if (!cycled) {
            directLength_ *= 2;
        }
        uniqueStopCount_ = std::unordered_set<bus_stop*>(stops.begin(), stops.end()).size();
    }
    void setLength(int meters) {
        length_ = meters;
        curvature_ = (double) length_ / directLength_;
    }
    const std::string& name() const {
        return name_;
    }
    const std::vector<bus_stop*> stops() const {
        return stops_;
    }
    bool isCycled() const{
        return isCycled_;
    }
    int stopsCount() const {
        return (isCycled_?stops_.size():stops_.size()*2-1);
    }
    int uniqueStopCount() const {
        return uniqueStopCount_;
    }
    int length() const {
        return length_;
    }
    double curvature() const {
        return curvature_;
    }

private:
    std::string name_;
    std::vector<bus_stop*> stops_;
    bool isCycled_ = false;
    double directLength_ = 0;
    int length_ = 0;
    double curvature_ = 1.0;
    int uniqueStopCount_ = 0;
};

std::ostream& operator<<(std::ostream& out, const bus_route& route);

struct busRouteComparator
{
    bool operator() (const bus_route* lhs, const bus_route* rhs) const {
        return std::lexicographical_compare(lhs->name().begin(), lhs->name().end(),
                                            rhs->name().begin(), rhs->name().end());
    }
};

using route_set = std::set<bus_route*,busRouteComparator>;

struct route_info {
    std::string_view name = ""s;
    int stopCount = 0;
    int uniqueStopCount = 0;
    int length = 0;
    double curvature = 1.0;
};

std::ostream& operator<<(std::ostream& out, const route_info& route);

struct stop_info {
    std::string_view name = ""s;
    route_set routes;
};

std::ostream& operator<<(std::ostream& out, const stop_info& stop);

struct NumberHasher {
    size_t operator() (const std::string_view& text) const {
        size_t res = 0;
        for (int i = 0; i < (int)text.size(); ++i) {
            res += text[i]-'A';
            res *= 100;
        }
        return res;
    }
};

struct StopHasher {
    size_t operator() (const std::string_view& text) const {
        size_t res = 0;
        for (int i = 0; i < (text.size() > 4?4:(int)text.size()); ++i) {
            res += text[i]-'A';
            res *= 100;
        }
        res *= text.size();
        return res;
    }
};

struct StopPairHasher {
    size_t operator() (const std::pair<bus_stop*,bus_stop*>& stops) const {
        return hasher(stops.first->name()) + hasher(stops.second->name());
    }
    StopHasher hasher;
};

class transport_catalogue {
public:
    transport_catalogue(){}
    ~transport_catalogue() {
        name_to_bus_.clear();
        name_to_stop_.clear();
        stop_to_buses_.clear();
        while (!routes_.empty()) {
            delete routes_.back();
            routes_.pop_back();
        }
        while (!stops_.empty()) {
            delete stops_.back();
            stops_.pop_back();
        }
    }
    void addStop(bus_stop* stop) {
        stops_.push_back(stop);
        name_to_stop_[stop->name()] = stop;
        if (stop_to_buses_.find(stop) == stop_to_buses_.end()) {
            stop_to_buses_[stop] = {};
        }
    }
    void addStop(std::string_view name, Coordinates place) {
        auto* stop = new bus_stop(name, place);
        //std::cout<<*stop<<std::endl;
        addStop(stop);
    }
    void addRoute(bus_route* route) {
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
    void addRoute(std::string_view name, std::vector<bus_stop*> stops, bool cycled = false) {
        auto* route = new bus_route(name, stops, cycled);
        //std::cout<<*route<<std::endl;
        addRoute(route);
    }
    void setDistance(bus_stop* first, bus_stop* second, int meters) {
        if ((first!= nullptr) && (second != nullptr)) {
            stops_to_distances_[{first, second}] = meters;
        } else {
            throw std::invalid_argument("Invalid stop"s);
        }
    }
    void setDistance(const std::string_view& first, const std::string_view& second, int meters) {
        if ((name_to_stop_.find(first) == name_to_stop_.end()) || (name_to_stop_.find(second) == name_to_stop_.end())) {
            throw std::invalid_argument("Invalid stop"s);
        }
        setDistance(name_to_stop_[first], name_to_stop_[second], meters);
    }

    bus_route* route(const std::string_view& name) const {
        return name_to_bus_.at(name);
    }
    bus_stop* stop(const std::string_view& name) const {
        return name_to_stop_.at(name);
    }
    route_set stopToBuses(const std::string_view& name) const {
        return stop_to_buses_.at(name_to_stop_.at(name));
    }
    route_set stopToBuses(bus_stop* stop) const {
        return stop_to_buses_.at(stop);
    }
    route_info routeInfo(const std::string_view& name) const {
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
    stop_info stopInfo(const std::string_view& name) const {
        stop_info result;
        result.name = name;
        if (stop_to_buses_.find(name_to_stop_.at(name)) == stop_to_buses_.end()) {
            throw std::out_of_range("Stop not found");
        };
        result.routes = stop_to_buses_.at(name_to_stop_.at(name));
        return result;
    }
    void printDistances() const{
        for (auto [stops, meters]: stops_to_distances_) {
            std::cout << stops.first->name() << " to "s << stops.second->name() << " "s << meters << "m"s <<std::endl;
        }
    }
    int distanceBetween(bus_stop* first, bus_stop* second) const {
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
    int distanceBetween(const std::string_view& first, const std::string_view& second) const {
        if ((name_to_stop_.find(first) == name_to_stop_.end()) || (name_to_stop_.find(second) == name_to_stop_.end())) {
            throw std::invalid_argument("Invalid stop"s);
        }
        return distanceBetween(name_to_stop_.at(first), name_to_stop_.at(second));
    }

private:
    std::deque<bus_stop*> stops_;
    std::deque<bus_route*> routes_;
    std::unordered_map<std::string_view, bus_route*, NumberHasher> name_to_bus_;
    std::unordered_map<std::string_view, bus_stop*, StopHasher> name_to_stop_;
    std::unordered_map<bus_stop*, route_set> stop_to_buses_;
    std::unordered_map<std::pair<bus_stop*,bus_stop*>,int, StopPairHasher> stops_to_distances_;
};

