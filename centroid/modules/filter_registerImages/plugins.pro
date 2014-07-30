TEMPLATE = lib
CONFIG += plugin
QT += widgets
INCLUDEPATH += ..
HEADERS = filter_registerImages.h
SOURCES = filter_registerImages.cpp
TARGET = $$qtLibraryTarget(_filter_registerImages)
DESTDIR = ../plugins

# install
target.path = ../plugins
INSTALLS += target
