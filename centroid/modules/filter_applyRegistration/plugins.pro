TEMPLATE = lib
CONFIG += plugin
QT += widgets
INCLUDEPATH += ..
HEADERS = filter_applyRegistration.h
SOURCES = filter_applyRegistration.cpp
TARGET = $$qtLibraryTarget(_filter_applyRegistration)
DESTDIR = ../plugins

# install
target.path = ../plugins
INSTALLS += target
