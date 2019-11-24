TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        custom_timer.c \
        ecgprotocol.c \
        main.c \
        radio.c

HEADERS += \
    custom_timer.h \
    ecgprotocol.h \
    radio.h
