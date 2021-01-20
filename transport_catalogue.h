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

class transport_stop {
public:
    transport_stop(std::string_view name, Coordinates place)
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

std::ostream& operator<<(std::ostream& out, const transport_stop& stop);

bool operator<(const transport_stop& lhs, const transport_stop& rhs);

class bus_route {
public:
    bus_route(std::string_view name, std::vector<transport_stop*> stops, bool cycled = false)
        :name_(name), stops_(stops), isCycled_(cycled) {
        length_ = 0;
        for (int i = 0; i < (int)stops.size()-1; ++i) {
            length_ += ComputeDistance(stops[i]->place(),stops[i+1]->place());
        }
        if (!isCycled()) {
            length_ *= 2;
        }
        uniqueStopCount_ = std::unordered_set<transport_stop*>(stops.begin(), stops.end()).size();
    }

    const std::string& name() const {
        return name_;
    }
    const std::vector<transport_stop*> stops() const {
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
    double length() const {
        return length_;
    }

private:
    std::string name_;
    std::vector<transport_stop*> stops_;
    bool isCycled_ = false;
    double length_ = 0;
    int uniqueStopCount_ = 0;
};

std::ostream& operator<<(std::ostream& out, const bus_route& route);

bool operator<(const bus_route& lhs, const bus_route& rhs);

struct busRouteComparator
{
    bool operator() (const bus_route* lhs, const bus_route* rhs) const
    {
        return *lhs < *rhs;
    }
};

using route_set = std::set<bus_route*,busRouteComparator>;

struct route_info {
    std::string_view name = ""s;
    int stopCount = 0;
    int uniqueStopCount = 0;
    double length = 0;
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

struct StringHasher {
size_t operator() (const std::string_view& text) const {
        //return hasher(std::string(text));
        return hasher(text);
    }
std::hash<std::string_view> hasher;
};

class transport_catalogue
{
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
    void addStop(transport_stop* stop) {
        stops_.push_back(stop);
        name_to_stop_[stop->name()] = stop;
        if (stop_to_buses_.find(stop) == stop_to_buses_.end()) {
            stop_to_buses_[stop] = {};
        }
    }
    void addStop(std::string_view name, Coordinates place) {
        auto* stop = new transport_stop(name, place);
        //std::cout<<*stop<<std::endl;
        addStop(stop);
    }
    void addRoute(bus_route* route) {
        routes_.push_back(route);
        name_to_bus_[route->name()] = route;
        for (auto * stop: route->stops()) {
            //std::cout<<stop->name()<<"|"s;
            stop_to_buses_[stop].insert(route);
        }
    }
    void addRoute(std::string_view name, std::vector<transport_stop*> stops, bool cycled = false) {
        auto* route = new bus_route(name, stops, cycled);
        //std::cout<<*route<<std::endl;
        addRoute(route);
    }
    bus_route* route(const std::string_view& name) const {
        return name_to_bus_.at(name);
    }
    transport_stop* stop(const std::string_view& name) const {
        return name_to_stop_.at(name);
    }
   route_set stopToBuses(const std::string_view& name) const {
        return stop_to_buses_.at(name_to_stop_.at(name));
    }
    route_set stopToBuses(transport_stop* stop) const {
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
private:
    std::deque<transport_stop*> stops_;
    std::deque<bus_route*> routes_;
    std::unordered_map<std::string_view, bus_route*, NumberHasher> name_to_bus_;
    std::unordered_map<std::string_view, transport_stop*, StopHasher> name_to_stop_;
    std::unordered_map<transport_stop*, route_set> stop_to_buses_;
};

