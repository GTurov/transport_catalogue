#include "json_reader.h"
#include "transport_catalogue.h"

#include "json.h"
#include <fstream>
#include <chrono>
#include <cassert>

using namespace transport;

//#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
//#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
//#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
//#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
//#define LOG_DURATION_STREAM(x, y) LogDuration UNIQUE_VAR_NAME_PROFILE(x, y)

//class LogDuration {
//public:
//    using Clock = std::chrono::steady_clock;

//    LogDuration(const std::string_view& id, std::ostream& out = std::cerr)
//        : id_(id), out_(out) {
//    }

//    ~LogDuration() {
//        using namespace std::chrono;
//        using namespace std::literals;
//        const auto end_time = Clock::now();
//        const auto dur = end_time - start_time_;
//        out_ << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
//    }

//private:
//    const std::string id_;
//    std::ostream& out_;
//    const Clock::time_point start_time_ = Clock::now();
//};

int main()
{
    std::ifstream l("out.json");
    json::Node lhs = json::Load(l).GetRoot();

    std::ifstream r("out.json");
    json::Node rhs = json::Load(r).GetRoot();

    //std::cerr<<lhs.GetRoot().AsArray().size();
    assert(lhs.AsArray().size() == rhs.AsArray().size());

    for (int i = 0; i < (int)lhs.AsArray().size(); ++i) {

    }


    for



    Catalogue catalogue;
    JsonReader jreader(catalogue);
    jreader.processQueries();

    return 0;
}
