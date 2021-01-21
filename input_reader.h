#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <stdexcept>
#include <string>


std::vector<std::string_view> splitIntoWords(const std::string_view& text);

std::vector<std::string_view> split(const std::string_view& text, char delimeter = ' ');

class input_reader
{
public:
    input_reader(transport_catalogue& catalogue)
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
    transport_catalogue& catalogue_;
};
