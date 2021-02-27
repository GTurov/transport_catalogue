#pragma once

#include "geo.h"
#include "route.h"

#include <string>
#include <set>

using namespace std::literals;

namespace transport {

class Stop {
public:
    Stop(const std::string_view name, const geo::Coordinates place)
        :name_(name), place_(place) {}
    const geo::Coordinates place() const {return place_;}
    const std::string& name() const {return name_;}
    struct Info {
        std::string name = ""s;
        RouteSet routes;
    };

private:
    std::string name_;
    geo::Coordinates place_;
};

std::ostream& operator<<(std::ostream& out, const Stop& stop);

std::ostream& operator<<(std::ostream& out, const Stop::Info& stop);

using StopSet = detail::ObjectWithNameSet<Stop>;

} // namespace transport
