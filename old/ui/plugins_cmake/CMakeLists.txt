cmake_minimum_required(VERSION 2.8.10)

project(centroid)

# set (ITK_DIR /usr/local/HEV/external/itk/build) 
set (ITK_DIR /home/tpb1/Downloads/InsightToolkit-4.5.0/build)
set (VTK_DIR /home/tpb1/Downloads/VTK-6.1.0/build)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

mark_as_advanced(CMAKE_BUILD_TYPE)
mark_as_advanced(CMAKE_INSTALL_PREFIX)
mark_as_advanced(CMAKE_OSX_ARCHITECTURES)
mark_as_advanced(CMAKE_OSX_DEPLOYMENT_TARGET)
mark_as_advanced(CMAKE_OSX_SYSROOT)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${CMAKE_BINARY_DIR}/../..)
include(GenerateExportHeader)

add_subdirectory(src)
add_subdirectory(modules)
