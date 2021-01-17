TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        input_reader.cpp \
        main.cpp \
        stat_reader.cpp \
        transport_catalogue.cpp

HEADERS += \
    geo.h \
    input_reader.h \
    stat_reader.h \
    transport_catalogue.h
