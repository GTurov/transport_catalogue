#include "test_example_functions.h"
#include "json_reader.h"
#include "geo.h"

#include <algorithm>
#include <chrono>
#include <deque>
//#include <execution>
//#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define PROFILE_CONCAT_INTERNAL(X, Y) X ## Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)

template <typename Func>
void RunTestImpl(Func& func, const string& func_name) {
    func();
    cerr << func_name << " OK" << endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)

template <typename UnitOfTime>
class AssertDuration {
public:
    using Clock = std::chrono::steady_clock;

    AssertDuration(int64_t max_duration, const std::string file, const std::string function, unsigned line)
        : max_dur_(max_duration)
        , file_(file)
        , function_(function)
        , line_(line) {
    }

    ~AssertDuration() {
        const auto dur = Clock::now() - start_time_;
        const auto converted_dur = std::chrono::duration_cast<UnitOfTime>(dur).count();
        if (converted_dur > max_dur_) {
            cerr << "Assert duration fail: "s << file_ << " "s << function_ << ": "s << line_ << endl;
            cerr << "Process duration is "s << converted_dur << " while max duration is " << max_dur_ << endl;
            cerr << "So the function worked longer on "s << converted_dur - max_dur_ << endl;
            abort();
        }
    }

private:
    int64_t max_dur_;
    std::string file_;
    std::string function_;
    unsigned line_;
    const Clock::time_point start_time_ = Clock::now();
};

#define ASSERT_DURATION_MILLISECONDS(x) AssertDuration<std::chrono::milliseconds> UNIQUE_VAR_NAME_PROFILE(x, __FILE__, __FUNCTION__, __LINE__)
#define ASSERT_DURATION_SECONDS(x) AssertDuration<std::chrono::seconds> UNIQUE_VAR_NAME_PROFILE(x, __FILE__, __FUNCTION__, __LINE__)

static const std::string file_path = ""s;//"C:\\Users\\aasir\\source\\repos\\aasirotkin\\TransportCatalogue\\Tests\\"s;
static const std::string file_path_out = ""s;//file_path + "output\\"s;

void SaveFile(const std::string& path, const std::string& file_name, const std::string& file_data) {
    std::ofstream file(path + file_name);
    file << file_data;
    file.close();
}

#define SAVE_FILE(name, file_data) (SaveFile(file_path_out, name, file_data))

static const std::string stop_prefix = "stop_"s;
static const std::string bus_prefix = "bus_"s;

std::string CreateStop(std::mt19937& generator, const std::string& stop_name) {
    std::string stop;
    stop += "\"type\": \"Stop\",\n"s;
    stop += "\"name\": \"" + stop_prefix + stop_name + "\",\n"s;

    double latitude = std::uniform_real_distribution<double>(-90.0, 90.0)(generator);
    double longitude = std::uniform_real_distribution<double>(-180.0, 180.0)(generator);

    stop += "\"latitude\":"s + std::to_string(latitude) + ",\n"s;
    stop += "\"longitude\":"s + std::to_string(longitude) + ",\n"s;

    return stop;
}

void AddRoadDistances(std::mt19937& generator, std::string& current_stop, int max_stop_name) {
    std::string road_distances = "\"road_distances\": {"s;
    bool first = true;
    for (int i = 0; i < max_stop_name; ++i) {
        if (!first) {
            road_distances += ", "s;
        }
        int distance = std::uniform_int_distribution<int>(100, 10000)(generator);
        road_distances += "\"" + stop_prefix + std::to_string(i) + "\": "s + std::to_string(distance);
        first = false;
    }
    road_distances += "}"s;
    current_stop += road_distances;
}

std::vector<std::string> CreateStops(std::mt19937& generator, int count) {
    std::vector<std::string> stops;
    for (int i = 0; i < count; ++i) {
        stops.push_back(CreateStop(generator, std::to_string(i)));
        AddRoadDistances(generator, stops.back(), i + 1);
    }
    return stops;
}

std::string CreateBus(std::mt19937& generator, const std::string& name, const std::vector<std::string>& stops, int stops_in_route_count) {
    std::string bus;
    bus += "\"type\": \"Bus\",\n"s;
    bus += "\"name\": \"" + bus_prefix + name + "\",\n"s;
    bus += "\"stops\": [";

    bool is_roundtrip = uniform_int_distribution<size_t>(0, 1)(generator);
    size_t stops_count = uniform_int_distribution<size_t>(0, stops_in_route_count)(generator);
    std::optional<size_t> first_stop;
    bool first = true;
    for (size_t i = 0; i < stops_count; ++i) {
        if (!first) {
            bus += ", "s;
        }
        size_t index = uniform_int_distribution<size_t>(0, stops.size() - 1)(generator);
        bus += "\"" + stop_prefix + std::to_string(index) + "\""s;
        first = false;

        if (!first_stop) {
            first_stop = index;
        }
    }
    if (is_roundtrip && first_stop) {
        bus += ", \""s + stop_prefix + std::to_string(*first_stop) + "\""s;
    }
    bus += "],\n"s;
    if (is_roundtrip) {
        bus += "\"is_roundtrip\": true"s;
    }
    else {
        bus += "\"is_roundtrip\": false"s;
    }
    return bus;
}

std::vector<std::string> CreateBuses(std::mt19937& generator, const std::vector<std::string>& stops, int bus_count, int max_stops_in_route) {
    std::vector<std::string> buses;
    for (int i = 0; i < bus_count; ++i) {
        buses.push_back(CreateBus(generator, std::to_string(i), stops, max_stops_in_route));
    }
    return buses;
}

std::string CreateBaseRequests(std::mt19937& generator, const std::vector<std::string>& buses, const std::vector<std::string>& stops) {
    std::string request;

    int bus_counter = 0;
    bool is_bus_valid = !buses.empty();
    int stop_counter = 0;
    bool is_stop_valid = !stops.empty();

    bool first = true;
    while (true) {
        if (!first) {
            request += ",\n";
        }
        first = false;

        int which_turn = std::uniform_int_distribution<int>(0, 1)(generator);

        if ((which_turn == 0 || !is_stop_valid) && is_bus_valid) {
            request += "{\n"s + buses.at(bus_counter++) + "\n}";
            is_bus_valid = !(bus_counter == (int)buses.size());
        }
        else if (is_stop_valid) {
            request += "{\n"s + stops.at(stop_counter++) + "\n}";
            is_stop_valid = !(stop_counter == (int)stops.size());
        }

        if (!is_bus_valid && !is_stop_valid) {
            break;
        }
    }

    return request;
}

std::string CreateRenderSettings() {
    std::string render_settings =
        "\"width\": 1000,\n"s +
        "\"height\": 1000,\n"s +
        "\"padding\": 30,\n"s +
        "\"stop_radius\": 5,\n"s +
        "\"line_width\": 14,\n"s +
        "\"bus_label_font_size\": 20,\n"s +
        "\"bus_label_offset\": [7, 15],\n"s +
        "\"stop_label_font_size\": 20,\n"s +
        "\"stop_label_offset\": [7, -3],\n"s +
        "\"underlayer_color\": [255, 255, 255, 0.85],\n"s +
        "\"underlayer_width\": 3,\n"s +
        "\"color_palette\": [\"green\", [255, 160, 0], \"red\", [255, 111, 3, 4]]"s;
    return render_settings;
}

std::string CreateStatRequest(std::mt19937 generator, int request_count, int stop_count, int bus_count) {
    std::string request;
    bool first = true;
    for (int i = 0; i < request_count; ++i) {
        if (!first) {
            request += ",\n";
        }
        first = false;

        if (i == 0 || i == (request_count / 2)) {
            request += "{ \"id\": " + std::to_string(i) + ", \"type\": \"Map\" }";
        }
        else if (i % 2 == 0) {
            int stop_id = std::uniform_int_distribution<int>(0, stop_count - 1)(generator);
            request += "{ \"id\": " + std::to_string(i) + ", \"type\": \"Stop\", \"name\": \"" + stop_prefix + std::to_string(stop_id) + "\" }"s;
        }
        else {
            int bus_id = std::uniform_int_distribution<int>(0, bus_count - 1)(generator);
            request += "{ \"id\": " + std::to_string(i) + ", \"type\": \"Bus\", \"name\": \"" + bus_prefix + std::to_string(bus_id) + "\" }"s;
        }
    }
    return request;
}

void TestRandomValues() {
    std::mt19937 generator;
    generator.seed();

    int stop_count = 100;
    int bus_count = 100;
    int stops_in_route_count = 100;

    // счётчик числа запросов
    int request_count = 2000;

    std::vector<std::string> stops(CreateStops(generator, stop_count));
    std::vector<std::string> buses(CreateBuses(generator, stops, bus_count, stops_in_route_count));

    std::string request = "{\n"s;

    request += "\"base_requests\": [\n"s;
    request += CreateBaseRequests(generator, buses, stops);
    request += "\n],\n"s;

    request += "\"render_settings\": {\n"s;
    request += CreateRenderSettings();
    request += "\n},\n"s;

    request += "\"stat_requests\": [\n"s;
    request += CreateStatRequest(generator, request_count, stop_count, bus_count);
    request += "]\n}";

    // Раскомментировать, если нужно сохранить файл с запросами
    // file_path_out - путь сохранения, переменная описана выше
    SAVE_FILE("request.json"s, request);

    std::stringstream in;
    std::stringstream out;
    in << request;

    {
        ASSERT_DURATION_SECONDS(2);

        // Место для вашего кода
        transport::Catalogue cat;

        json_reader jreader(cat);

        jreader.process_queries(in, out);
    }

    // Раскомментировать, если нужно сохранить файл с результатами
    // file_path_out - путь сохранения, переменная описана выше
    //SAVE_FILE("request_result.txt"s, out.str());
}

// --------- Окончание модульных тестов -----------

// Функция TestTransportCatalogue является точкой входа для запуска тестов
void TestTransportCatalogue() {
    RUN_TEST(TestRandomValues);
}
