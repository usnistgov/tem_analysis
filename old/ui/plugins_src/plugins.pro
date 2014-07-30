TEMPLATE = lib
CONFIG += plugin
QT += widgets
INCLUDEPATH += ..
HEADERS = filter_invert.h
SOURCES = filter_invert.cpp
TARGET = $$qtLibraryTarget(_filter_invert)
DESTDIR = ../plugins

# install
target.path = ../plugins
INSTALLS += target
