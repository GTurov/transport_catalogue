#include "transport_catalogue.h"

#include <ostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& out, const bus_stop& stop) {
    out << "Stop "s << stop.name() <<": "s;
    out << std::setprecision(6);
    out << stop.place().lat << " "s << stop.place().lng;
    return out;
}

bool operator<(const bus_stop& lhs, const bus_stop& rhs) {
    return std::lexicographical_compare(lhs.name().begin(), lhs.name().end(),
                                        rhs.name().begin(), rhs.name().end());
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
