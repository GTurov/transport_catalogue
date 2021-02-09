#pragma once

#include "transport_catalogue.h"

#include <iostream>

class json_reader
{
public:
    json_reader(transport::Catalogue& catalogue)
        :catalogue_(catalogue) {}
    void process_queries(std::istream& in = std::cin, std::ostream& out = std::cout);
private:
    transport::Catalogue& catalogue_;
};

