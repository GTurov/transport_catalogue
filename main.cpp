#include "input_reader.h"
#include "json_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>
#include <iomanip> // Why???

using namespace std;
using namespace transport;

int main()
{
    Catalogue catalogue;
    input_reader reader(catalogue);
    json_reader jreader(catalogue);

    stringstream  inputData;
    inputData << "18"s << endl
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
              << "Stop Prazhskaya: 55.611678, 37.603831"s << endl
              << "Bus 100500: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s << endl
              << "Bus 921a: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s << endl
              << "Bus 828a: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s << endl
              << "Bus A: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s << endl
              << "Bus b: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s << endl
                 ;
    stringstream  queryData;
    queryData << "6"s << endl
              << "Bus 256"s << endl
              << "Bus 750"s << endl
              << "Bus 751"s << endl
              << "Stop Samara"s << endl
              << "Stop Prazhskaya"s << endl
              << "Stop Biryulyovo Zapadnoye"s << endl;


    stringstream input;
    input << "13"s << endl
          << "Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino"s << endl
          << "Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino"s << endl
          << "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye"s << endl
          << "Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka"s << endl
          << "Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino"s << endl
          << "Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam"s << endl
          << "Stop Biryusinka: 55.581065, 37.64839, 750m to Universam"s << endl
          << "Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya"s << endl
          << "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya"s << endl
          << "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye"s << endl
          << "Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye"s << endl
          << "Stop Rossoshanskaya ulitsa: 55.595579, 37.605757"s << endl
          << "Stop Prazhskaya: 55.611678, 37.603831"s << endl
          << "6"s << endl
          << "Bus 256"s << endl
          << "Bus 750"s << endl
          << "Bus 751"s << endl
          << "Stop Samara"s << endl
          << "Stop Prazhskaya"s << endl
          << "Stop Biryulyovo Zapadnoye"s << endl;


    //reader.fillDatabase(cin);
    //reader.fillDatabase(input);
    jreader.process_queries();
    //cout<<"-------------"s<<endl;
    //reader.readQueries(cin);
    //reader.readQueries(queryData);
    //reader.readQueries(input);

//    cout<<"-------------"s<<endl;
//    catalogue.printDistances();
//    cout<<catalogue.distanceBetween("Tolstopaltsevo"s,"Marushkino"s)<<std::endl;
//    cout<<catalogue.distanceBetween("Marushkino"s,"Tolstopaltsevo"s)<<std::endl;
//    cout<<catalogue.distanceBetween("Marushkino"s,"Marushkino"s)<<std::endl;
//    cout<<catalogue.distanceBetween("Rossoshanskaya ulitsa"s,"Biryulyovo Zapadnoye"s)<<std::endl;
//    cout<<catalogue.distanceBetween("Biryulyovo Zapadnoye"s, "Rossoshanskaya ulitsa"s)<<std::endl;
//    cout<<catalogue.routeInfo("256"s)<<endl;
//    cout<<catalogue.routeInfo("750"s)<<endl;
    //cout<<catalogue.routeInfo("114"s)<<endl;
//    try {
//        auto r = catalogue.routeInfo("751"s);
//        cout<<r<<endl;
//    }  catch (exception e) {
//        cout<<"not found"<<endl;
//    }
//    cout<<catalogue.routeInfo("751"s)<<endl;


    return 0;
}
