#pragma once

#include "geo.h"

#include <deque>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace transport {

using namespace std::literals;

class Stop;

class Route {
public:
    Route(std::string_view name, std::vector<Stop*> stops,
              bool cycled = false);
    void setLength(int meters) {
        length_ = meters;
        curvature_ = (double) length_ / directLength_;
    }
    const std::string& name() const {return name_;}
    const std::vector<Stop*> stops() const {return stops_;}
    bool isCycled() const{return isCycled_;}
    int stopsCount() const {return (isCycled_?stops_.size():stops_.size()*2-1);}
    int uniqueStopCount() const {return uniqueStopCount_;}
    int length() const {return length_;}
    double curvature() const {return curvature_;}
    struct Info {
        std::string_view name = ""s;
        int stopCount = 0;
        int uniqueStopCount = 0;
        int length = 0;
        double curvature = 1.0;
    };

private:
    std::string name_;
    std::vector<Stop*> stops_;
    bool isCycled_ = false;
    double directLength_ = 0;
    int length_ = 0;
    double curvature_ = 1.0;
    int uniqueStopCount_ = 0;
};

std::ostream& operator<<(std::ostream& out, const Route& route);

std::ostream& operator<<(std::ostream& out, const Route::Info& route);


namespace detail {

struct RouteComparator {
    bool operator() (const Route* lhs, const Route* rhs)const ;
};

struct RouteNumberHasher {
    size_t operator() (const std::string_view& text) const;
};

struct StopNameHasher {
    size_t operator() (const std::string_view& text) const;
};

struct StopPairHasher {
    size_t operator() (const std::pair<Stop*,Stop*>& stops) const;
};

}

using RouteSet = std::set<Route*, detail::RouteComparator>;

class Stop {
public:
    Stop(std::string_view name, Coordinates place)
        :name_(name), place_(place)  {}
    const Coordinates place() const {return place_;}
    const std::string& name() const {return name_;}
    struct Info {
        std::string_view name = ""s;
        RouteSet routes;
    };

private:
    std::string name_;
    Coordinates place_;
};

std::ostream& operator<<(std::ostream& out, const Stop& stop);

std::ostream& operator<<(std::ostream& out, const Stop::Info& stop);

class Catalogue {
public:
    Catalogue(){}
    ~Catalogue();
    void addStop(Stop* stop);
    void addStop(std::string_view name, Coordinates place);
    void addRoute(Route* route);
    void addRoute(std::string_view name, std::vector<Stop*> stops, bool cycled = false);
    void setDistance(Stop* first, Stop* second, int meters);
    void setDistance(const std::string_view& first, const std::string_view& second, int meters);
    Route* route(const std::string_view& name) const {return name_to_bus_.at(name);}
    Stop* stop(const std::string_view& name) const {return name_to_stop_.at(name);}
    RouteSet stopToBuses(const std::string_view& name) const {return stop_to_buses_.at(name_to_stop_.at(name));}
    RouteSet stopToBuses(Stop* stop) const {return stop_to_buses_.at(stop);}
    Route::Info routeInfo(const std::string_view& name) const;
    Stop::Info stopInfo(const std::string_view& name) const;
    int distanceBetween(Stop* first, Stop* second) const;
    int distanceBetween(const std::string_view& first, const std::string_view& second) const;

private:
    std::deque<Stop*> stops_;
    std::deque<Route*> routes_;
    std::unordered_map<std::string_view, Route*, detail::RouteNumberHasher> name_to_bus_;
    std::unordered_map<std::string_view, Stop*, detail::StopNameHasher> name_to_stop_;
    std::unordered_map<Stop*, RouteSet> stop_to_buses_;
    std::unordered_map<std::pair<Stop*,Stop*>, int, detail::StopPairHasher> stops_to_distances_;
};

}
