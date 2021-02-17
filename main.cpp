#include "json_reader.h"
#include "transport_catalogue.h"

using namespace transport;

int main()
{
    Catalogue catalogue;
    json_reader jreader(catalogue);
    jreader.process_queries();
    return 0;
}
