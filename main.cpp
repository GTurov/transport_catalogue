#include "json_reader.h"
#include "transport_catalogue.h"

#include "json.h"
#include <fstream>
#include <chrono>
#include <cassert>

using namespace transport;

#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define LOG_DURATION_STREAM(x, y) LogDuration UNIQUE_VAR_NAME_PROFILE(x, y)

class LogDuration {
public:
    using Clock = std::chrono::steady_clock;

    LogDuration(const std::string_view& id, std::ostream& out = std::cerr)
        : id_(id), out_(out) {
    }

    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;
        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        out_ << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

private:
    const std::string id_;
    std::ostream& out_;
    const Clock::time_point start_time_ = Clock::now();
};

int main()
{
//    std::ifstream l("out.json");
//    json::Node lhs = json::Load(l).GetRoot();

//    std::ifstream r("e4_output.json");
//    json::Node rhs = json::Load(r).GetRoot();

//    //std::cerr<<lhs.GetRoot().AsArray().size();
//    assert(lhs.AsArray().size() == rhs.AsArray().size());

//    const json::Array& left = lhs.AsArray();
//    const json::Array& right = rhs.AsArray();
//    for (int i = 0; i < (int)lhs.AsArray().size(); ++i) {
//        if (left[i].AsDict().find("total_time"s) != left[i].AsDict().end()) {
//            if (std::abs(
//                        left[i].AsDict().at("total_time"s).AsDouble() -
//                        right[i].AsDict().at("total_time"s).AsDouble()
//                        ) > left[i].AsDict().at("total_time"s).AsDouble()/10000) {
//                std::cout<<left[i]<<std::endl;
//                std::cout<<right[i]<<std::endl<<std::endl;
//            }
//        } else {
//            if (left[i] != right[i]) {
//                std::cout<<left[i]<<std::endl;
//                std::cout<<right[i]<<std::endl<<std::endl;
//            }
//        }
//    }

    {
        LOG_DURATION("Total"sv);

        Catalogue catalogue;
        JsonReader jreader(catalogue);
        jreader.processQueries();
    }
    return 0;
}
