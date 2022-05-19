# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 2.9)
project(report_compiler)

# Executable to expose the report compiler to external users through terminal.

set(${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    )

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

# add lib-common-lib.a dependency
target_link_libraries(${PROJECT_NAME} PRIVATE hebench_common-lib)

target_link_libraries(${PROJECT_NAME} PRIVATE hebench_report_compiler)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
