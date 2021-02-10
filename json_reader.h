#pragma once

#include "transport_catalogue.h"
#include "json.h"

#include <iostream>
#include <string>

json::Node makeStopAnswer(int request_id, transport::Stop::Info data);
json::Node makeRouteAnswer(int request_id, transport::Route::Info data);

enum class request_type {
    REQUEST_STOP, REQUEST_BUS
};

struct request {
    int id;
    request_type type;
    std::string name;
};

struct stopAnswer {
    int id = 0;
    transport::Stop::Info answer;
    std::string error = "";
};

class json_reader
{
public:
    json_reader(transport::Catalogue& catalogue)
        :catalogue_(catalogue) {}
    void process_queries(std::istream& in = std::cin, std::ostream& out = std::cout);
private:

private:
    transport::Catalogue& catalogue_;
};

