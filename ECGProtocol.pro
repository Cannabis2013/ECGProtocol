TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        ecgprotocol.c \
        main.c \
        radio.c

HEADERS += \
    ecgprotocol.h \
    radio.h
