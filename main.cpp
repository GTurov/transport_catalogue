#include "json_reader.h"
#include "transport_catalogue.h"

using namespace transport;

int main()
{
    Catalogue catalogue;
    JsonReader jReader(catalogue);
    jReader.processQueries();
    return 0;
}
