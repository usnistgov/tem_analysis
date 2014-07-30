project(filter_registerImages)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

add_definitions(-DQT_PLUGIN)

add_compiler_export_flags()

add_library(${PROJECT_NAME} SHARED
  filter_registerImages.cpp
  filter_registerImages.h
)

qt5_use_modules(${PROJECT_NAME} Core)
qt5_use_modules(${PROJECT_NAME} Gui)
qt5_use_modules(${PROJECT_NAME} Widgets)

find_package (ITK REQUIRED)
if(ITK_FOUND)
  include( ${ITK_USE_FILE} ) 
endif(ITK_FOUND)

target_link_libraries(${PROJECT_NAME} ${ITK_LIBRARIES})
