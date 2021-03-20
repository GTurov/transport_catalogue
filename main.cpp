#include "json_reader.h"
#include "transport_catalogue.h"

#include "log_duration.h"

using namespace transport;

int main()
{
    {
        LOG_DURATION("Total"sv);
        std::iostream null (0);

        Catalogue catalogue;
        JsonReader jreader(catalogue);
        //jreader.processQueries(std::cin, null);
        jreader.processQueries();
    }
    return 0;
}
