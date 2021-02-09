#include "domain.h"

#include <unordered_set>
#include <iomanip>

namespace transport {

Route::Route(const std::string_view name, const std::vector<Stop *>& stops, bool cycled)
    :name_(name), stops_(stops), isCycled_(cycled) {
    for (int i = 0; i < (int)stops.size()-1; ++i) {
        directLength_ += ComputeDistance(stops[i]->place(),stops[i+1]->place());
    }
    if (!cycled) {
        directLength_ *= 2;
    }
    uniqueStopCount_ = std::unordered_set<Stop*>(stops.begin(), stops.end()).size();
}

bool detail::RouteComparator::operator() (const Route* lhs, const Route* rhs) const {
    return std::lexicographical_compare(
                lhs->name().begin(), lhs->name().end(),
                rhs->name().begin(), rhs->name().end());
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

std::ostream& operator<<(std::ostream& out, const Stop& stop) {
    out << "Stop "s << stop.name() <<": "s;
    out << std::setprecision(6);
    out << stop.place().lat << " "s << stop.place().lng;
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

} // transport
