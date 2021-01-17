#include "input_reader.h"
#include "transport_catalogue.h"

#include <iostream>

using namespace std;

int main()
{
    transport_stop stop1 {"Vokzal"s,{123.45,678.90}};
    transport_stop stop3 {"Aeroport"s,{243.00,476.00}};
    transport_stop stop2 {"Zavod"s,{143.00,676.00}};
    cout<<stop1<<endl<<stop2<<endl<<stop3<<endl;
    bus_route bus1 {"1a"s, {&stop1, &stop2, &stop3}};
    cout<<bus1<<endl;
    route_info info1 {bus1.name(),5,3,100.45};
    cout<<info1;

    transport_catalogue catalogue;
    input_reader reader(catalogue);

    stringstream  inputData;
    inputData << "10"s <<
                 "Stop Tolstopaltsevo: 55.611087, 37.208290"s << endl <<
                 "Stop Marushkino: 55.595884, 37.209755"s << endl <<
                 "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s << endl <<
                 "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"s << endl <<
                 "Stop Rasskazovka: 55.632761, 37.333324"s << endl <<
                 "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700"s << endl <<
                 "Stop Biryusinka: 55.581065, 37.648390"s << endl <<
                 "Stop Universam: 55.587655, 37.645687"s << endl <<
                 "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656"s << endl <<
                 "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164"s << endl <<
                 "3"s << endl <<
                 "Bus 256"s << endl <<
                 "Bus 750"s << endl <<
                 "Bus 751"s << endl;

    //reader.readQueries(cin);
    reader.readQueries(inputData);

    return 0;
}
