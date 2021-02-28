TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        json.cpp \
        json_builder.cpp \
        json_reader.cpp \
        main.cpp \
        map_renderer.cpp \
        request_handler.cpp \
        route.cpp \
        stat_reader.cpp \
        stop.cpp \
        svg.cpp \
        transport_catalogue.cpp

HEADERS += \
    geo.h \
    json.h \
    json_builder.h \
    json_reader.h \
    map_renderer.h \
    request_handler.h \
    route.h \
    stat_reader.h \
    stop.h \
    svg.h \
    transport_catalogue.h

DISTFILES += \
    coi.json \
    etalone.svg \
    etalone_map.json \
    input_rus.json \
    my.svg
