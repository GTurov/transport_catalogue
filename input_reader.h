#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace transport {

namespace detail {
std::vector<std::string_view> split(const std::string_view& text, char delimeter = ' ');
}

class input_reader
{
public:
    input_reader(Catalogue& catalogue)
        :catalogue_(catalogue) {}
    void fillDatabase(std::istream& input);
    void readQueries(std::istream& input);

private:
    struct distance {
        std::string_view from;
        std::string_view to;
        int meters;
    };
    distance ParseDistance(std::string_view stopName, std::string_view distanceData);

private:
    Catalogue& catalogue_;
};

}
