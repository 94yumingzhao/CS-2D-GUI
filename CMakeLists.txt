# CS-2D-GUI - Qt6 GUI for 2D Cutting Stock Problem Solver
# 二维下料问题求解器图形界面

cmake_minimum_required(VERSION 3.24)

project(CS-2D-GUI
    VERSION 1.0.0
    DESCRIPTION "Qt6 GUI for 2D Cutting Stock Problem Solver"
    LANGUAGES CXX
)

# C++ Standard (与 CS-2D-BP-Arc 保持一致)
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

# CPLEX
set(CPLEX_DIR "D:/CPLEX" CACHE PATH "CPLEX installation directory")
if(NOT EXISTS ${CPLEX_DIR})
    message(FATAL_ERROR "CPLEX directory not found at ${CPLEX_DIR}. Please set CPLEX_DIR correctly.")
endif()

# MSVC Runtime
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

#---------------------------------------
# CS-2D-BP-Arc Core Library (from source)
#---------------------------------------

set(CORE_PROJECT_DIR "${CMAKE_SOURCE_DIR}/../CS-2D-BP-Arc" CACHE PATH "Path to CS-2D-BP-Arc project")
if(NOT EXISTS ${CORE_PROJECT_DIR}/src/2DBP.h)
    message(FATAL_ERROR "CS-2D-BP-Arc project not found at ${CORE_PROJECT_DIR}")
endif()

# 核心库源文件 (排除 main.cpp)
set(CORE_SOURCES
    ${CORE_PROJECT_DIR}/src/logger.cpp
    ${CORE_PROJECT_DIR}/src/input.cpp
    ${CORE_PROJECT_DIR}/src/output.cpp
    ${CORE_PROJECT_DIR}/src/heuristic.cpp
    ${CORE_PROJECT_DIR}/src/arc_flow.cpp
    ${CORE_PROJECT_DIR}/src/root_node.cpp
    ${CORE_PROJECT_DIR}/src/root_node_sub.cpp
    ${CORE_PROJECT_DIR}/src/column_generation.cpp
    ${CORE_PROJECT_DIR}/src/new_node.cpp
    ${CORE_PROJECT_DIR}/src/new_node_sub.cpp
    ${CORE_PROJECT_DIR}/src/branch_and_price.cpp
)

set(CORE_HEADERS
    ${CORE_PROJECT_DIR}/src/2DBP.h
    ${CORE_PROJECT_DIR}/src/logger.h
)

add_library(CuttingStockCore STATIC ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(CuttingStockCore PUBLIC
    ${CORE_PROJECT_DIR}/src
    "${CPLEX_DIR}/cplex/include"
    "${CPLEX_DIR}/concert/include"
)

target_compile_definitions(CuttingStockCore PUBLIC IL_STD)

target_link_directories(CuttingStockCore PUBLIC
    "${CPLEX_DIR}/cplex/lib/x64_windows_msvc14/stat_mda"
    "${CPLEX_DIR}/concert/lib/x64_windows_msvc14/stat_mda"
)

target_link_libraries(CuttingStockCore PUBLIC
    cplex2210
    ilocplex
    concert
)

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
    src/cutting_view_dialog.cpp
)

set(GUI_HEADERS
    src/main_window.h
    src/solver_worker.h
    src/parameter_widget.h
    src/results_widget.h
    src/log_widget.h
    src/cutting_view_widget.h
    src/cutting_view_dialog.h
)

qt_add_executable(CS-2D-GUI
    ${GUI_SOURCES}
    ${GUI_HEADERS}
)

target_link_libraries(CS-2D-GUI PRIVATE
    CuttingStockCore
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
message(STATUS "Qt6 Version: ${Qt6_VERSION}")
message(STATUS "CPLEX Directory: ${CPLEX_DIR}")
message(STATUS "Core Project: ${CORE_PROJECT_DIR}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "==========================================")
message(STATUS "")
