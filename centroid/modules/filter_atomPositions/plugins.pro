TEMPLATE = lib
CONFIG += plugin
QT += widgets
INCLUDEPATH += ..
HEADERS = filter_atomPositions.h
SOURCES = filter_atomPositions.cpp
TARGET = $$qtLibraryTarget(_filter_atomPositions)
DESTDIR = ../plugins

# install
target.path = ../plugins
INSTALLS += target
