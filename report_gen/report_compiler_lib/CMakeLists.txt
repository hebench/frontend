# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 2.9)
project(hebench_report_compiler)

# Shared Library to expose the report compiler capabilities.

set(${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_report_compiler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_report_stats.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_report_overview_header.cpp"
    )

set(${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_report_compiler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_report_stats.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_report_overview_header.h"
    )

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

# add lib-common-lib.a dependency
target_link_libraries(${PROJECT_NAME} PRIVATE hebench_common-lib)

# add the report generation libraries dependency

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(${PROJECT_NAME} PRIVATE hebench_reportgen_lib)
target_link_libraries(${PROJECT_NAME} PRIVATE hebench_reportgen)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
