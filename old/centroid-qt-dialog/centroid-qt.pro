QT += core gui widgets

TEMPLATE = app
TARGET = centroid-qt
INCLUDEPATH += .
# INCLUDEPATH += /home/tpb1/Code/Tomasz/nano-tubes/centroids/centroid-qt/VTK-6.1.0/install/include/vtk-6.1
INCLUDEPATH += /usr/include/vtk

# LIBS += -L/home/tpb1/Code/Tomasz/nano-tubes/centroids/centroid-qt/VTK-6.1.0/install/lib -lvtkHybrid -lvtkWidgets 
LIBS += -L/usr/bin/vtk -lvtkHybrid -lvtkWidgets 

SOURCES += centroid-qt.cpp
FORMS += particles_form.ui
  