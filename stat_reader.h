#pragma once

#include "transport_catalogue.h"

//class stat_reader
//{
//public:
//    stat_reader(transport_catalogue& catalogue)
//        :catalogue_(catalogue){}
//void readQueries(std::istream& input) {
////    for (int i = 0; i < 3; ++i) {
////        std::string line;
////              std::getline(input, line);
////              std::cout<<line<<std::endl;
////    }
////    return;
//    int query_count;
//    input >> query_count ;
//    //std::cout<<query_count;
//    std::vector<std::string> raw_queries;
//    for (int i = 0; i < query_count; ++i) {
//        std::string line;
//        std::getline(input, line);
//        size_t first_space = line.find_first_of(" "s);
//        std::string_view command = line.substr(0,first_space);

//        if(command == "Bus"sv) {
//            std::vector<std::string_view> words = SplitIntoWords(command);
//            std::cout<<"Bus "s<<line.substr(first_space)<<std::endl;
//            std::cout<<catalogue_.routeInfo(line.substr(first_space))<<std::endl;
//        }
//    }
//}

//private:
//    transport_catalogue& catalogue_;
//};
