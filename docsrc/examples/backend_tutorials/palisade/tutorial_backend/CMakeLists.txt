# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.12)
project(be_tutorial LANGUAGES C CXX)

# C++ version
set(CMAKE_CXX_STANDARD 17) # C++ standard C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tutorial_engine_palisade.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tutorial_eltwiseadd_benchmark_palisade.cpp"
    )
set(${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_error_palisade.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_engine_palisade.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_eltwiseadd_benchmark_palisade.h"
    )

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# All external libraries are assumed to be pre-compiled for tutorial simplicity.

# find the hebench_cpp archive
set(HEBENCH_API_INCLUDE_DIR "/include/directory/for/api_bridge")
target_include_directories(${PROJECT_NAME} PRIVATE ${HEBENCH_API_INCLUDE_DIR}) # point to include for api_bridge
find_library(hebench_cpp_FOUND NAMES libhebench_cpp.a HINTS "/directory/containing/libhebench_cpp.a")
if(hebench_cpp_FOUND)
    add_library(hebench_cpp UNKNOWN IMPORTED)
    # populate the found library with its properties
    set_property(TARGET hebench_cpp PROPERTY IMPORTED_LOCATION ${hebench_cpp_FOUND})
    set_property(TARGET hebench_cpp APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${HEBENCH_API_INCLUDE_DIR})
else()
    message(FATAL_ERROR "libhebench_cpp.a not found.")
endif()

# link found library
target_link_libraries(${PROJECT_NAME} PUBLIC "-Wl,--whole-archive" hebench_cpp "-Wl,--no-whole-archive")

# find other third party dependencies
# pre-built PALISADE
set(PALISADE_HINT_DIR "/usr/local/")

find_package(Palisade HINTS ${PALISADE_HINT_DIR} REQUIRED)
add_library(libpalisade INTERFACE)
target_include_directories(libpalisade INTERFACE
    ${OPENMP_INCLUDES} # Will be empty if PALISADE is built single-threaded
    ${PALISADE_INCLUDE}
    ${PALISADE_INCLUDE}/third-party/include
    ${PALISADE_INCLUDE}/core
    ${PALISADE_INCLUDE}/pke
    ${PALISADE_INCLUDE}/binfhe)
target_link_directories(libpalisade INTERFACE ${PALISADE_LIBDIR})
target_link_libraries(libpalisade INTERFACE ${PALISADE_LIBRARIES} ${OPENMP_LIBRARIES})

# Ignore errors from PALISADE
add_compile_options(-Wno-error=unused-parameter -Wno-error=ignored-qualifiers -Wno-error=deprecated-copy)

# link found library
target_link_libraries(${PROJECT_NAME} PUBLIC libpalisade)

# extra compile options
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra) # show warnings

# install options
include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
