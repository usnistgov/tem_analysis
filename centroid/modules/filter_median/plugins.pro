TEMPLATE = lib
CONFIG += plugin
QT += widgets
INCLUDEPATH += ..
HEADERS = filter_median.h
SOURCES = filter_median.cpp
TARGET = $$qtLibraryTarget(_filter_median)
DESTDIR = ../plugins

# install
target.path = ../plugins
INSTALLS += target
