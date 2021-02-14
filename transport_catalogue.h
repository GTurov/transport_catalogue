#pragma once

#include "domain.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

namespace transport {

namespace detail {

struct RouteNumberHasher {
    size_t operator() (const std::string_view text) const;
};

struct StopNameHasher {
    size_t operator() (const std::string_view text) const;
};

struct StopPairHasher {
    size_t operator() (const std::pair<Stop*,Stop*> stops) const;
};

} // detail

class Catalogue {
public:
    Catalogue(){}
    ~Catalogue();
    void addStop(Stop* stop);
    void addStop(const std::string_view name, geo::Coordinates place);
    void addRoute(Route* route);
    void addRoute(const std::string_view name, std::vector<Stop*> stops, bool cycled = false);
    void setDistance(Stop* first, Stop* second, int meters);
    void setDistance(const std::string_view first, const std::string_view second, int meters);
    Route* route(const std::string_view name) const {return name_to_bus_.at(name);}
    Stop* stop(const std::string_view name) const {return name_to_stop_.at(name);}
    const RouteSet allRoutes() const {return RouteSet(routes_.begin(), routes_.end());}
    const StopSet allStops() const {return StopSet(stops_.begin(), stops_.end());}
    const RouteSet routesViaStop(const std::string_view name) const {return stop_to_buses_.at(name_to_stop_.at(name));}
    const Route::Info routeInfo(const std::string_view name) const;
    const Stop::Info stopInfo(const std::string_view name) const;
    int distanceBetween(Stop* first, Stop* second) const;
    int distanceBetween(const std::string_view first, const std::string_view second) const;

private:
    std::deque<Stop*> stops_;
    std::deque<Route*> routes_;
    std::unordered_map<std::string_view, Route*, detail::RouteNumberHasher> name_to_bus_;
    std::unordered_map<std::string_view, Stop*, detail::StopNameHasher> name_to_stop_;
    std::unordered_map<Stop*, RouteSet> stop_to_buses_;
    std::unordered_map<std::pair<Stop*,Stop*>, int, detail::StopPairHasher> stops_to_distances_;
};

} // transport
