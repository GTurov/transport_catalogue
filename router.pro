TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        domain.cpp \
        json.cpp \
        json_builder.cpp \
        json_reader.cpp \
        main.cpp \
        map_renderer.cpp \
        svg.cpp \
        transport_catalogue.cpp \
        transport_router.cpp

HEADERS += \
    domain.h \
    geo.h \
    graph.h \
    json.h \
    json_builder.h \
    json_reader.h \
    map_renderer.h \
    ranges.h \
    router.h \
    svg.h \
    transport_catalogue.h \
    transport_router.h

DISTFILES += \
    coi.json \
    e4_input.json \
    e4_output.json \
    etalone.svg \
    etalone_map.json \
    example1.json \
    example4out.json \
    input_rus.json \
    my.svg \
    out.json
