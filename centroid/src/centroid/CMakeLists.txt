project(centroid)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
cmake_policy(SET CMP0020 NEW) # automatically link to qtmain.lib on Windows

qt5_wrap_ui(UIS_HDRS
   forms/main_window.ui
   forms/main_viewer.ui
   forms/new_project.ui
)

qt5_add_resources(RES_SRC main_window.qrc)

add_executable(${PROJECT_NAME} main.cpp main_window.cpp modules_manager.cpp block.cpp project.cpp metadata.cpp module_button.cpp main_viewer.cpp viewer_graphics_scene.cpp new_project.cpp particles.cpp particle_item.cpp ${UIS_HDRS} ${RES_SRC})

qt5_use_modules(${PROJECT_NAME} Core)
qt5_use_modules(${PROJECT_NAME} Gui)
qt5_use_modules(${PROJECT_NAME} Widgets)

find_package (ITK REQUIRED)
if(ITK_FOUND)
   include( ${ITK_USE_FILE} ) 
endif(ITK_FOUND)
target_link_libraries(${PROJECT_NAME} ${ITK_LIBRARIES})

find_package (VTK REQUIRED)
if(VTK_FOUND)
   include( ${VTK_USE_FILE} ) 
endif(VTK_FOUND)
target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})

