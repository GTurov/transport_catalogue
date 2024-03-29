#pragma once

#include <cmath>

namespace geo {

struct Coordinates {
    double lat;
    double lng;
};

inline double computeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    static const double dr = 3.1415926535 / 180.;
    static const double EarthRadius = 6371000.0;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * EarthRadius;
}

} // geo
