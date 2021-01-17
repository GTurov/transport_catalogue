#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std::literals;

class transport_stop {
public:
    transport_stop(std::string_view name, Coordinates place)
        :name_(name), place_(place)  {}
    const Coordinates place() const {
        return place_;
    }
    const std::string_view name() const {
        return name_;
    }
private:
    std::string name_;
    Coordinates place_;
};

std::ostream& operator<<(std::ostream& out, const transport_stop& stop);

class bus_route {
public:
    bus_route(std::string_view name, std::vector<transport_stop*> stops)
        :name_(name), stops_(stops) {
        isCycled_ = (stops_[0]->name() == stops_[stops_.size()-1]->name());
    }

    std::string_view name() const {
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
        return (isCycled_?stops_.size()-1:stops_.size());
    }


private:
    std::string name_;
    std::vector<transport_stop*> stops_;
    bool isCycled_;
};

std::ostream& operator<<(std::ostream& out, const bus_route& route);

struct route_info {
    std::string_view name;
    int stopCount;
    int uniqueStopCount;
    double length;
};

std::ostream& operator<<(std::ostream& out, const route_info& route);

class transport_catalogue
{
public:
    transport_catalogue(){}
    ~transport_catalogue() {} // Clear memory!!!
    void addStop(transport_stop* stop) {
        stops_.push_back(stop);
        stop_to_place_[stop->name()] = stop;
    }
    void addRoute(bus_route* route) {
        routes_.push_back(route);
        bus_to_route_[route->name()] = route;
    }
    bus_route* route(const std::string_view& name) const;
    transport_stop* stop(const std::string_view& name) const {
        return stop_to_place_.at(name);
    }
    route_info routeInfo(const std::string_view& name) const;
private:
    std::deque<transport_stop*> stops_;
    std::deque<bus_route*> routes_;
    std::unordered_map<std::string_view,bus_route*> bus_to_route_;
    std::unordered_map<std::string_view,transport_stop*> stop_to_place_;
};

