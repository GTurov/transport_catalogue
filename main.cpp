#include "json_reader.h"
#include "transport_catalogue.h"

using namespace transport;

int main()
{
    Catalogue catalogue;
    JsonReader jreader(catalogue);
    jreader.processQueries();

    return 0;
}
