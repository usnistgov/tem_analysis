project(filter_subset)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

add_definitions(-DQT_PLUGIN)
cmake_policy(SET CMP0020 NEW) # automatically link to qtmain.lib on Windows

add_compiler_export_flags()

add_library(${PROJECT_NAME} SHARED
  filter_subset.cpp
  filter_subset.h
)

qt5_use_modules(${PROJECT_NAME} Core)
qt5_use_modules(${PROJECT_NAME} Gui)
qt5_use_modules(${PROJECT_NAME} Widgets)

find_package (ITK REQUIRED)
if(ITK_FOUND)
  include( ${ITK_USE_FILE} ) 
endif(ITK_FOUND)

target_link_libraries(${PROJECT_NAME} ${ITK_LIBRARIES})
