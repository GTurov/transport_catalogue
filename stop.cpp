#include "stop.h"

#include <iomanip>

namespace transport {

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

} // namespace transport
