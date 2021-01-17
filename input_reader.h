#pragma once

#include "transport_catalogue.h"

#include <istream>
#include <string>


#include <iostream>

std::vector<std::string_view> SplitIntoWords(const std::string_view& text);

class input_reader
{
public:
    input_reader(transport_catalogue& catalogue)
        :catalogue_(catalogue) {}
    void readQueries(std::istream& input) {
        int query_count;
        input >> query_count ;
        std::vector<std::string> raw_queries;
        for (int i = 0; i < query_count; ++i) {
            std::string line;
            std::getline(input, line);
            raw_queries.push_back(std::move(line));
        }
        //std::cout<<std::endl<<"total: "<<raw_queries.size()<<std::endl;
        std::vector<std::string_view> stop_raw_queries;
        std::vector<std::string_view> route_raw_queries;
        for (int i = 0; i < (int)raw_queries.size(); ++i) {
            //std::cout<< raw_queries[i]<<std::endl;
            std::string_view line = raw_queries[i];
            if(line.substr(0,5) == "Stop "sv) {
                stop_raw_queries.push_back(line);
            }
            if(line.substr(0,4) == "Bus "sv) {
                route_raw_queries.push_back(line);
            }
        }
        std::cout<<"Stops:"<<std::endl;
        for (int i = 0; i < (int)stop_raw_queries.size(); ++i) {
            //std::cout<<stop_raw_queries[i].substr(5)<<std::endl;
            transport_stop * stop = makeStop(stop_raw_queries[i].substr(5));
            catalogue_.addStop(stop);
            std::cout<<*stop<<std::endl;
        }
        std::cout<<"Routes:"<<std::endl;
        for (int i = 0; i < (int)route_raw_queries.size(); ++i) {
            //std::cout<<route_raw_queries[i].substr(4)<<std::endl;
            bus_route * route = makeBusRoute(route_raw_queries[i].substr(4));
        }
    }

private:
    transport_stop * makeStop(std::string_view command) {
        std::vector<std::string_view> words = SplitIntoWords(command);
//        for(auto w: words) {
//            std::cout<<w<<'-';
//        }
//        std::cout<<std::endl;
//        return nullptr;
        return new transport_stop(words[0],{std::stod(std::string(words[1])),
                    std::stod(std::string(words[2]))});
    }
    bus_route * makeBusRoute(std::string_view command) {
        return nullptr;
    }
private:
    transport_catalogue& catalogue_;
};

