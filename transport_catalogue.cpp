#include "transport_catalogue.h"

#include <ostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& out, const transport_stop& stop) {
    out << "Stop "s << stop.name() <<": "s;
    out << std::setprecision(6);
    out << stop.place().lat << " "s << stop.place().lng;
    return out;
}


std::ostream& operator<<(std::ostream& out, const bus_route& route) {
    out << "Bus "s << route.name() <<": "s;
    bool firstStop = true;
    for (const transport_stop* stop: route.stops()) {
        out << (firstStop?""s:" > "s)<< stop->name();
        firstStop = false;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const route_info& route) {
    out << "Bus "s << route.name <<": "s;
    out << route.stopCount << " stops on route, "s;
    out << route.uniqueStopCount << " unique stops, "s;
    out << route.length << " route length"s;
    return out;
}
