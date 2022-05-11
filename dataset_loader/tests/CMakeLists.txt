# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.12)

project(hebench_dataset_loader_test LANGUAGES C CXX)

set(${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/hebench_dataset_loader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/hebench_dataset_loader_functional_test.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/main.cpp"
    )
set(${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/hebench_dataset_loader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/hebench_dataset_loader_functional_test.h"
    )

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# set compiler properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_link_libraries(${PROJECT_NAME} PRIVATE dl)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
