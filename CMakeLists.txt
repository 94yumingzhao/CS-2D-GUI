# CS-2D-GUI - Qt6 GUI for 2D Cutting Stock Problem Solver
# This project launches CS-2D-BP-Arc.exe as a subprocess

cmake_minimum_required(VERSION 3.24)

project(CS-2D-GUI
    VERSION 2.1.0
    DESCRIPTION "Qt6 GUI for 2D Cutting Stock Problem Solver"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt6 Auto tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

#---------------------------------------
# Dependencies
#---------------------------------------

# Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets Core)
qt_standard_project_setup()

# MSVC Runtime
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

#---------------------------------------
# GUI Executable
#---------------------------------------

set(GUI_SOURCES
    src/main.cpp
    src/main_window.cpp
    src/solver_worker.cpp
    src/parameter_widget.cpp
    src/results_widget.cpp
    src/log_widget.cpp
    src/cutting_view_widget.cpp
    src/generator_widget.cpp
    src/generator_worker.cpp
    src/difficulty_mapper.cpp
)

set(GUI_HEADERS
    src/main_window.h
    src/solver_worker.h
    src/parameter_widget.h
    src/results_widget.h
    src/log_widget.h
    src/cutting_view_widget.h
    src/generator_widget.h
    src/generator_worker.h
    src/difficulty_mapper.h
)

qt_add_executable(CS-2D-GUI
    ${GUI_SOURCES}
    ${GUI_HEADERS}
)

target_link_libraries(CS-2D-GUI PRIVATE
    Qt6::Widgets
    Qt6::Core
)

set_target_properties(CS-2D-GUI PROPERTIES
    WIN32_EXECUTABLE TRUE
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)

#---------------------------------------
# Summary
#---------------------------------------

message(STATUS "")
message(STATUS "=== CS-2D-GUI Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Qt6 Version: ${Qt6_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Solver: ../CS-2D-BP-Arc/build/release/bin/Release/CS-2D-BP-Arc.exe")
message(STATUS "Generator: ../CS-2D-Data/build/release/bin/Release/CS-2D-Data.exe")
message(STATUS "==========================================")
message(STATUS "")
