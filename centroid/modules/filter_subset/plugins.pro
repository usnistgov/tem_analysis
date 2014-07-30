TEMPLATE = lib
CONFIG += plugin
QT += widgets
INCLUDEPATH += ..
HEADERS = filter_subset.h
SOURCES = filter_subset.cpp
TARGET = $$qtLibraryTarget(_filter_subset)
DESTDIR = ../plugins

# install
target.path = ../plugins
INSTALLS += target
