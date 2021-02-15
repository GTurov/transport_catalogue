#include "json_reader.h"
#include "transport_catalogue.h"

#include "test_example_functions.h"
#include "log_duration.h"


#include <sstream>
#include <cassert>
using namespace std;
using namespace transport;

void stringstream_assert(stringstream& lhs, stringstream& rhs) {
    std::string s1, s2;
    int str_num = 0;
    while (getline(lhs, s1) && getline(rhs, s2)) {
        if(s1 != s2) {
            std::cerr << ++str_num << std::endl;
            std::cerr<<s1<<std::endl<<s2<<std::endl;
            for(int i =0; i < (int)s1.size(); ++i) {
                std::cerr<<(s1[i]==s2[i]?' ':'^');
            }
            assert(false);
        }
    }
}


void json_answer_test(json_reader& reader){
    std::string json_input =
    "{"
      "\"base_requests\": ["
        "{"
          "\"type\": \"Bus\","
          "\"name\": \"114\","
          "\"stops\": [\"Mokriy voksal\", \"Rivierskiy most\"],"
          "\"is_roundtrip\": false"
        "},"
        "{"
          "\"type\": \"Stop\","
          "\"name\": \"Rivierskiy most\","
          "\"latitude\": 43.587795,"
          "\"longitude\": 39.716901,"
          "\"road_distances\": {\"Mokriy voksal\": 850}"
        "},"
        "{"
          "\"type\": \"Stop\","
          "\"name\": \"Mokriy voksal\","
          "\"latitude\": 43.581969,"
          "\"longitude\": 39.719848,"
          "\"road_distances\": {\"Rivierskiy most\": 850}"
        "}"
      "],"
      "\"render_settings\": {"
        "\"width\": 200,"
        "\"height\": 200,"
        "\"padding\": 30,"
        "\"stop_radius\": 5,"
        "\"line_width\": 14,"
        "\"bus_label_font_size\": 20,"
        "\"bus_label_offset\": [7, 15],"
        "\"stop_label_font_size\": 20,"
        "\"stop_label_offset\": [7, -3],"
        "\"underlayer_color\": [255,255,255,0.85],"
        "\"underlayer_width\": 3,"
        "\"color_palette\": [\"green\", [255,160,0],\"red\"]"
      "},"
      "\"stat_requests\": ["
        "{ \"id\": 1, \"type\": \"Map\" },"
        "{ \"id\": 2, \"type\": \"Stop\", \"name\": \"Rivierskiy most\" },"
        "{ \"id\": 3, \"type\": \"Bus\", \"name\": \"114\" }"
      "]"
    "}";

    //std::string json_etalone ="{ \"map\": \"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Mokriy voksal</text>\n  <text fill=\"black\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Mokriy voksal</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Rivierskiy most</text>\n  <text fill=\"black\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Rivierskiy most</text>\n</svg>\"\n\"}";
    //std::string json_etalone ="{ \"map\": \"<?xml version=\\\"1.0\\\" encoding=\\\"UTF-8\\\" ?>\\n<svg xmlns=\\\"http://www.w3.org/2000/svg\\\" version=\\\"1.1\\\">\\n  <polyline points=\\\"100.817,170 30,30 100.817,170\\\" fill=\\\"none\\\" stroke=\\\"green\\\" stroke-width=\\\"14\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\"/>\\n  <text fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\" x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\">114</text>\\n  <text fill=\\\"green\\\" x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\">114</text>\\n  <text fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\" x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\">114</text>\\n  <text fill=\\\"green\\\" x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\">114</text>\\n  <circle cx=\\\"100.817\\\" cy=\\\"170\\\" r=\\\"5\\\" fill=\\\"white\\\"/>\\n  <circle cx=\\\"30\\\" cy=\\\"30\\\" r=\\\"5\\\" fill=\\\"white\\\"/>\\n  <text fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\" x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\">Mokriy voksal</text>\\n  <text fill=\\\"black\\\" x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\">Mokriy voksal</text>\\n  <text fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\" x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\">Rivierskiy most</text>\\n  <text fill=\\\"black\\\" x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\">Rivierskiy most</text>\\n</svg>\\\"\\n\"}";
    //std::cerr << json_etalone;
    std::string json_etalone =
            "[\n"
                "{\n"
                    "\"map\": \"<?xml version=\\\"1.0\\\" encoding=\\\"UTF-8\\\" ?>\\n<svg xmlns=\\\"http://www.w3.org/2000/svg\\\" version=\\\"1.1\\\">\\n  <polyline points=\\\"100.817,170 30,30 100.817,170\\\" fill=\\\"none\\\" stroke=\\\"green\\\" stroke-width=\\\"14\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\"/>\\n  <text fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\" x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\">114</text>\\n  <text fill=\\\"green\\\" x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\">114</text>\\n  <text fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\" x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\">114</text>\\n  <text fill=\\\"green\\\" x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"15\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\" font-weight=\\\"bold\\\">114</text>\\n  <circle cx=\\\"100.817\\\" cy=\\\"170\\\" r=\\\"5\\\" fill=\\\"white\\\"/>\\n  <circle cx=\\\"30\\\" cy=\\\"30\\\" r=\\\"5\\\" fill=\\\"white\\\"/>\\n  <text fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\" x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\">Mokriy voksal</text>\\n  <text fill=\\\"black\\\" x=\\\"100.817\\\" y=\\\"170\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\">Mokriy voksal</text>\\n  <text fill=\\\"rgba(255,255,255,0.85)\\\" stroke=\\\"rgba(255,255,255,0.85)\\\" stroke-width=\\\"3\\\" stroke-linecap=\\\"round\\\" stroke-linejoin=\\\"round\\\" x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\">Rivierskiy most</text>\\n  <text fill=\\\"black\\\" x=\\\"30\\\" y=\\\"30\\\" dx=\\\"7\\\" dy=\\\"-3\\\" font-size=\\\"20\\\" font-family=\\\"Verdana\\\">Rivierskiy most</text>\\n</svg>\""
                    "\"request_id\": 1\n"
                "},\n"
                "{\n"
                    "\"buses\": [\n"
                        "\"114\"\n"
                    "],\n"
                    "\"request_id\": 2\n"
                "},\n"
                "{\n"
                    "\"curvature\": 1.23199,\n"
                    "\"request_id\": 3,\n"
                    "\"route_length\": 1700,\n"
                    "\"stop_count\": 3,\n"
                    "\"unique_stop_count\": 2\n"
                "}\n"
            "]\n";

    std::string json_output;
    std::stringstream in(json_input);
    std::stringstream etalone(json_etalone);
    std::stringstream out(json_output);

    reader.process_queries(in, out);
    json::Document my_answer = json::Load(out);
    //json::Print(my_answer,std::cerr);
    //std::cerr<<my_answer.GetRoot().AsArray().at(0).AsMap().at("map"s).AsString();
    //std::cerr << std::endl << std::endl;
    json::Document etalone_answer = json::Load(etalone);
    //std::cerr<<etalone_answer.GetRoot().AsArray().at(0).AsMap().at("map"s).AsString();
    //json::Print(etalone_answer,std::cerr);
  //  std::cerr << std::endl << std::endl;
//    assert(etalone_answer.GetRoot().AsArray().at(0).AsMap().at("map"s).AsString()==my_answer.GetRoot().AsArray().at(0).AsMap().at("map"s).AsString());
    assert(my_answer == etalone_answer);
    stringstream_assert(etalone, out);
    std::cerr << "Test OK\n";
}

int main()
{
    LOG_DURATION("processing"s);
    TestTransportCatalogue();
    //Catalogue catalogue;
    //json_reader jreader(catalogue);
    //jreader.process_queries();


    return 0;
}
