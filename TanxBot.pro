#-------------------------------------------------
#
# Project created by QtCreator 2015-07-31T18:56:27
#
#-------------------------------------------------

QT       += core websockets

QT       -= gui

TARGET = TankBot
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    tanxinterface.cpp \
    userinterface.cpp \
    tanxmap.cpp

HEADERS += \
    tanxinterface.h \
    userinterface.h \
    tanxmap.h
