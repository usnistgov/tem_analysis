QT += core gui widgets

TEMPLATE = app
TARGET = centroid

# INCLUDEPATH += /usr/include/vtk
# LIBS += -L/usr/bin/vtk -lvtkHybrid -lvtkWidgets 

SOURCES += main.cpp main_window.cpp modules_manager.cpp block.cpp module_button.cpp main_viewer.cpp viewer_graphics_scene.cpp graphics_items.cpp particles.cpp
HEADERS += main_window.h interface.h modules_manager.h block.h module_button.h main_viewer.h viewer_graphics_scene.h graphics_items.h particles.h
FORMS += main_window.ui main_viewer.ui
RESOURCES += main_window.qrc
