# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

project(generic_wl_backend)

set(generic_wl_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/generic_wl_engine.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/generic_wl_benchmark.cpp"
    )
set(generic_wl_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/generic_wl_engine.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/generic_wl_benchmark.h"
    )

add_library(${PROJECT_NAME} SHARED ${generic_wl_SOURCES} ${generic_wl_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# find the hebench_cpp archive
set(HEBENCH_API_BRIDGE_INSTAL_DIR "/install/directory/for/api_bridge")
set(HEBENCH_API_INCLUDE_DIR "${HEBENCH_API_BRIDGE_INSTAL_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE ${HEBENCH_API_INCLUDE_DIR}) # point to include for api_bridge
find_library(hebench_cpp_FOUND NAMES libhebench_cpp.a HINTS "${HEBENCH_API_BRIDGE_INSTAL_DIR}/lib")
if(hebench_cpp_FOUND)
    add_library(hebench_cpp UNKNOWN IMPORTED)
    # populate the found library with its properties
    set_property(TARGET hebench_cpp PROPERTY IMPORTED_LOCATION ${hebench_cpp_FOUND})
    set_property(TARGET hebench_cpp APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${HEBENCH_API_INCLUDE_DIR})
    message(STATUS "libhebench_cpp.a found")
else()
    message(FATAL_ERROR "libhebench_cpp.a not found.")
endif()

# link found library
target_link_libraries(${PROJECT_NAME} PUBLIC "-Wl,--whole-archive" hebench_cpp "-Wl,--no-whole-archive")

# extra compile options
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra) # show warnings

# install options
include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
install(TARGETS ${PROJECT_NAME} DESTINATION lib)

