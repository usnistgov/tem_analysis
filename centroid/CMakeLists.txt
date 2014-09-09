cmake_minimum_required(VERSION 2.8.10)

project(centroid)

if (WIN32)
    set(CMAKE_CXX_FLAGS "-mwindows")
endif()

set (ITK_DIR ${CMAKE_BINARY_DIR}/../../sdk/ITK-4.5.0/build)
set (VTK_DIR ${CMAKE_BINARY_DIR}/../../sdk/VTK-6.1.0/build)

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
cmake_policy(SET CMP0020 NEW) # automatically link to qtmain.lib on Windows

include_directories(${CMAKE_BINARY_DIR}/../src/centroid)
include(GenerateExportHeader)

add_subdirectory(modules)
add_subdirectory(src)
