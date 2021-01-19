#pragma once

#include "geo.h"

#include <deque>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <iostream>

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
        length_ = 0;
        for (int i = 0; i < (int)stops.size()-1; ++i) {
            length_ += ComputeDistance(stops[i]->place(),stops[i+1]->place());
        }
        if (!isCycled()) {
            length_ *= 2;
        }
        uniqueStopCount_ = std::unordered_set(stops.begin(),stops.end()).size();
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
        return uniqueStopCount_;
        //return (isCycled_?stops_.size()-1:stops_.size());
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

struct route_info {
    std::string_view name = ""s;
    int stopCount = 0;
    int uniqueStopCount = 0;
    double length = 0;
};

std::ostream& operator<<(std::ostream& out, const route_info& route);

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
        bus_to_route_.clear();
        stop_to_place_.clear();
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
        stop_to_place_[stop->name()] = stop;
    }
    void addStop(std::string_view name, Coordinates place) {
        auto* stop = new transport_stop(name, place);
        //std::cout<<*stop<<std::endl;
        addStop(stop);
    }
    void addRoute(bus_route* route) {
        routes_.push_back(route);
        bus_to_route_[route->name()] = route;
    }
    void addRoute(std::string_view name, std::vector<transport_stop*> stops) {
        auto* route = new bus_route(name, stops);
        //std::cout<<*route<<std::endl;
        addRoute(route);
    }
    bus_route* route(const std::string_view& name) const;
    transport_stop* stop(const std::string_view& name) const {
        return stop_to_place_.at(name);
    }
    route_info routeInfo(const std::string_view& name) const {
        route_info result;
        result.name = name;
        if (bus_to_route_.find(name) == bus_to_route_.end()) {
            return result;
        };
        bus_route * route = bus_to_route_.at(name);
        result.stopCount = route->stopsCount();
        result.uniqueStopCount = route->uniqueStopCount();
        result.length = route->length();
        return result;
    }
private:
    std::deque<transport_stop*> stops_;
    std::deque<bus_route*> routes_;
    std::unordered_map<std::string_view, bus_route*, NumberHasher> bus_to_route_;
    std::unordered_map<std::string_view, transport_stop*, StopHasher> stop_to_place_;
//    std::unordered_map<std::string_view, bus_route*, StringHasher> bus_to_route_;
//    std::unordered_map<std::string_view, transport_stop*, StringHasher> stop_to_place_;
//    std::unordered_map<std::string_view, bus_route*> bus_to_route_;
//    std::unordered_map<std::string_view, transport_stop*> stop_to_place_;
};

