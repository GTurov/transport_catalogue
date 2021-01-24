#include "input_reader.h"

namespace transport {

std::vector<std::string_view> detail::split(const std::string_view& text, char delimeter) {
    std::vector<std::string_view> words;
    size_t word_begin = 0;
    size_t word_end = 0;
    while(word_begin != std::string::npos ) {
        word_begin = text.find_first_not_of(delimeter, word_end);
        if (word_begin == std::string::npos) {
            break;
        }
        word_end = text.find_first_of(delimeter,word_begin);
        std::string_view tmp = text.substr(word_begin,word_end-word_begin);
        size_t true_begin = tmp.find_first_not_of(" "s);
        size_t true_end = tmp.find_last_not_of(" "s);
        words.push_back(tmp.substr(true_begin, true_end-true_begin+1));
    }
    return words;
}

void input_reader::fillDatabase(std::istream& input) {
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
    std::vector<std::string_view> stop_raw_queries;
    std::vector<std::string_view> route_raw_queries;

    for (int i = 0; i < (int)raw_queries.size(); ++i) {
        std::string_view line = raw_queries[i];
        size_t first_space = line.find(' ');
        std::string_view command = line.substr(0,first_space);
        //std::cout<<"'"s<<command<<"'"s<<std::endl; // debug
        if(command == "Stop"sv) {
            stop_raw_queries.push_back(line.substr(first_space+1));
        }
        if(command == "Bus"sv) {
            route_raw_queries.push_back(line.substr(first_space+1));
        }
    }

    // Stops
    std::vector<distance> distances;
    for (int i = 0; i < (int)stop_raw_queries.size(); ++i) {
        std::string_view line = stop_raw_queries[i];
        size_t doubledotPos = line.find(':');
        std::string_view stopName = line.substr(0,doubledotPos);
        line = line.substr(doubledotPos+2);
        std::vector<std::string_view> params = detail::split(line, ',');
        std::string_view latSv = params[0];
        std::string_view lngSv = params[1];
        catalogue_.addStop(stopName,{std::stod(std::string(latSv)),
                                     std::stod(std::string(lngSv))});
        for (int i = 2; i < (int)params.size(); ++i) {
            distances.push_back(ParseDistance(stopName, params[i]));
        }
    }

    // Distances
    for (const distance& d: distances) {
        catalogue_.setDistance(d.from, d.to, d.meters);
    }

    // Routes
    for (int i = 0; i < (int)route_raw_queries.size(); ++i) {
        size_t doubledotPos = route_raw_queries[i].find(':');
        std::string_view busName = route_raw_queries[i].substr(0,doubledotPos);

        std::vector<std::string_view> words;
        const std::string delimetrs = "->"s;
        size_t word_begin = doubledotPos+1;
        size_t word_end = doubledotPos+1;
        std::vector<Stop*> stops;
        bool isCycled = false;
        while(word_begin != std::string::npos ) {
            word_begin = route_raw_queries[i].find_first_not_of(delimetrs, word_end);
            if (word_begin == std::string::npos) {
                break;
            }
            word_end = route_raw_queries[i].find_first_of(delimetrs,word_begin);
            if (word_end != std::string::npos) {
                isCycled = (route_raw_queries[i][word_end] == '>')&&(word_end != std::string::npos);
            }
            std::string_view stopName =
                    route_raw_queries[i].substr(word_begin+1,word_end-word_begin
                                                - (word_end==std::string::npos?1:2));
            stops.push_back(catalogue_.stop(stopName));

        }
        catalogue_.addRoute(busName, std::move(stops), isCycled);
    }
}

void input_reader::readQueries(std::istream& input) {
    int query_count;
    input >> query_count;
    std::string str;
    std::getline(input,str);
    std::vector<std::string> raw_queries;
    for (int i = 0; i < query_count; ++i) {
        std::string line;
        std::getline(input, line);
        std::string_view sv = line;
        size_t first_space = sv.find(' ');
        std::string_view command = sv.substr(0,first_space);

        if (command == "Bus"sv) {
            std::string_view busName = sv.substr(first_space+1);
            Route::Info info = catalogue_.routeInfo(busName);
            std::cout<<info<<std::endl;
        }
        if (command == "Stop"sv) {
            std::string_view stopName = sv.substr(first_space+1);
            try {
                Stop::Info info = catalogue_.stopInfo(stopName);
                std::cout<<info<<std::endl;

            }  catch (const std::exception& e) {
                std::cout<<"Stop "s<<stopName<<": not found"s<<std::endl;
            }
        }
    }
}

input_reader::distance input_reader::ParseDistance(std::string_view stopName, std::string_view distanceData) {
    distance result;
    if (distanceData.empty()) {
        throw std::invalid_argument("Data is empty"s);
    }
    result.from = stopName;
    size_t mPos = distanceData.find('m');
    result.meters = std::stoi(std::string(distanceData.substr(0,mPos)));
    result.to = distanceData.substr(mPos+5);
    return result;
}

}
