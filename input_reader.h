#pragma once

#include "transport_catalogue.h"

#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string>


std::vector<std::string_view> SplitIntoWords(const std::string_view& text);

class input_reader
{
public:
    input_reader(transport_catalogue& catalogue)
        :catalogue_(catalogue) {}
    void fillDatabase(std::istream& input) {
        int query_count;
        input >> query_count ;
        std::string str;
        std::getline(input,str);
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
            size_t first_space = line.find_first_of(' ');
            std::string_view command = line.substr(0,first_space);
            //std::cout<<"'"s<<command<<"'"s<<std::endl;
            if(command == "Stop"sv) {
                //std::cout<<"Stop"s<<std::endl;
                stop_raw_queries.push_back(line.substr(first_space));
            }
            if(command == "Bus"sv) {
                //std::cout<<"Bus"s<<std::endl;
                route_raw_queries.push_back(line.substr(first_space));
            }
        }
        //std::cout<<"Stops:"<<std::endl;
        for (int i = 0; i < (int)stop_raw_queries.size(); ++i) {
            std::vector<std::string_view> words = SplitIntoWords(stop_raw_queries[i]);
            catalogue_.addStop(words[0],{std::stod(std::string(words[1])),
                        std::stod(std::string(words[2]))});
//            double lat, lng;
//            auto result = std::from_chars(words[1].data(), words[1].data() + words[1].size(), lat);
        }
        //std::cout<<"Routes:"<<std::endl;
        for (int i = 0; i < (int)route_raw_queries.size(); ++i) {
            std::vector<std::string_view> words = SplitIntoWords(route_raw_queries[i]);
            std::string_view name = words[0];
            std::vector<transport_stop*> stops;
            for (int i = 1; i < (int)words.size(); ++i) {
                stops.push_back(catalogue_.stop(words[i]));
            }
            catalogue_.addRoute(name, std::move(stops));
        }
    }

    void readQueries(std::istream& input) {
        int query_count;
        input >> query_count;
        std::string str;
        std::getline(input,str);
        //std::cout<<query_count;
        std::vector<std::string> raw_queries;
        for (int i = 0; i < query_count; ++i) {
            std::string line;
            std::getline(input, line);
            //std::cout<<"Bus "s<<i<<" "<<line<<std::endl;
            std::string_view sv = line;
            size_t first_space = sv.find_first_of(" "s);
            std::string_view command = sv.substr(0,first_space);

            if(command == "Bus"sv) {
                std::vector<std::string_view> words = SplitIntoWords(sv.substr(first_space));
                //std::cout<<"Bus "s<<words[0]<<std::endl;
                try {
                    route_info info = catalogue_.routeInfo(words[0]);
                    std::cout<<info<<std::endl;

                }  catch (const std::exception& e) {
                    std::cout<<"Bus "s<<words[0]<<": not found"s<<std::endl;
                }
            }
        }
    }


private:
    transport_catalogue& catalogue_;
};

