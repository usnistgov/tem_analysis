TEMPLATE = lib
CONFIG += plugin
QT += widgets
INCLUDEPATH += ..
HEADERS = filter_atomCorrelation.h
SOURCES = filter_atomCorrelation.cpp
TARGET = $$qtLibraryTarget(_filter_atomCorrelation)
DESTDIR = ../plugins

# install
target.path = ../plugins
INSTALLS += target
