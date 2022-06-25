cmake_minimum_required(VERSION 3.5)

project(voronoi_visualizer VERSION 0.1 LANGUAGES CXX)

# find_package(Boost REQUIRED COMPONENTS polygon)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS OpenGL)

include_directories("C:/Program Files/boost_1_79_0")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        voronoi_visual_utils.hpp
        GLWidget.h
        GLWidget.cpp
)

add_executable(voronoi_visualizer ${PROJECT_SOURCES})
#if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
#    qt_add_executable(voronoi_visualizer
#        MANUAL_FINALIZATION
#        ${PROJECT_SOURCES}
#    )
## Define target properties for Android with Qt 6 as:
##    set_property(TARGET voronoi_visualizer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
##                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
## For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
#else()
#    if(ANDROID)
#        add_library(voronoi_visualizer SHARED
#            ${PROJECT_SOURCES}
#        )
## Define properties for Android with Qt 5 after find_package() calls as:
##    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    else()
#        add_executable(voronoi_visualizer
#            ${PROJECT_SOURCES}
#        )
#    endif()
#endif()

target_link_libraries(voronoi_visualizer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt5::OpenGL)

set_target_properties(voronoi_visualizer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(voronoi_visualizer)
endif()
