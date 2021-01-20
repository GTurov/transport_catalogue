#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>
#include <iomanip> // Why???

using namespace std;

int main()
{
//    transport_stop stop1 {"Vokzal"s,{123.45,678.90}};
//    transport_stop stop3 {"Aeroport"s,{243.00,476.00}};
//    transport_stop stop2 {"Zavod"s,{143.00,676.00}};
//    cout<<stop1<<endl<<stop2<<endl<<stop3<<endl;
//    bus_route bus1 {"1a"s, {&stop1, &stop2, &stop3}};
//    cout<<bus1<<endl;
//    route_info info1 {bus1.name(),5,3,100.45};
//    cout<<info1;
//    cout<<endl<<"-------------"s<<endl;

    transport_catalogue catalogue;
    input_reader reader(catalogue);

    stringstream  inputData;
    inputData << "13"s << endl
              << "Stop Tolstopaltsevo: 55.611087, 37.208290"s << endl
              << "Stop Marushkino: 55.595884, 37.209755"s << endl
              << "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s << endl
              << "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka"s << endl
              << "Stop Rasskazovka: 55.632761, 37.333324"s << endl
              << "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700"s << endl
              << "Stop Biryusinka: 55.581065, 37.648390"s << endl
              << "Stop Universam: 55.587655, 37.645687"s << endl
              << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656"s << endl
              << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164"s << endl
              << "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s << endl
              << "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757"s << endl
              << "Stop Prazhskaya: 55.611678, 37.603831"s << endl;
    stringstream  queryData;
    queryData << "6"s << endl
              << "Bus 256"s << endl
              << "Bus 750"s << endl
              << "Bus 751"s << endl
              << "Stop Samara"s << endl
              << "Stop Prazhskaya"s << endl
              << "Stop Biryulyovo Zapadnoye"s << endl;


    //reader.fillDatabase(cin);
    reader.fillDatabase(inputData);
    //cout<<"-------------"s<<endl;
    //reader.readQueries(cin);
    reader.readQueries(queryData);
    //    cout<<"-------------"s<<endl;
    //    cout<<catalogue.routeInfo("256"s)<<endl;
    //    cout<<catalogue.routeInfo("750"s)<<endl;
    //    try {
    //        auto r = catalogue.routeInfo("751"s);
    //        cout<<r<<endl;
    //    }  catch (exception e) {
    //        cout<<"not found"<<endl;
    //    }
    //    cout<<catalogue.routeInfo("751"s)<<endl;


    return 0;
}
