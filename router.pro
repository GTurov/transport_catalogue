TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        domain.cpp \
        json.cpp \
        json_reader.cpp \
        main.cpp \
        map_renderer.cpp \
        request_handler.cpp \
        stat_reader.cpp \
        svg.cpp \
        test_example_functions.cpp \
        transport_catalogue.cpp

HEADERS += \
    domain.h \
    geo.h \
    json.h \
    json_reader.h \
    log_duration.h \
    map_renderer.h \
    request_handler.h \
    stat_reader.h \
    svg.h \
    test_example_functions.h \
    transport_catalogue.h

DISTFILES += \
    coi.json \
    etalone.svg \
    etalone_map.json \
    file1.json \
    file1.svg \
    file2.json \
    file2.svg \
    input.json \
    input_rus.json \
    my.svg \
    my_map.json \
    stop_label.svg
