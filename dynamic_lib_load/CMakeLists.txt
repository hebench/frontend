# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

project(hebench_dynamic_lib_load)

set(${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/dynamic_lib_load.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    )
set(${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/dynamic_lib_load.h"
    )

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE dl)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
