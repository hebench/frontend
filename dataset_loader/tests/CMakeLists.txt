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

set(COMMON_LIB_SRC "../../build/third-party/common-lib/common-lib-src")
set(COMMON_LIB_BUILD "../../build/third-party/common-lib/common-lib-build")

if(DEFINED ENV{COMMON_LIB_SRC} OR DEFINED ENV{COMMON_LIB_BUILD})
    set(COMMON_LIB_SRC $ENV{COMMON_LIB_SRC})
    set(COMMON_LIB_BUILD $ENV{COMMON_LIB_BUILD})
    message(STATUS "Modified Common Library Directories")
endif()

message(STATUS "Common Library Source Directory: ${COMMON_LIB_SRC}")
message(STATUS "Common Library Build Directory: ${COMMON_LIB_BUILD}")
add_subdirectory(${COMMON_LIB_SRC} ${COMMON_LIB_BUILD})

target_link_libraries(${PROJECT_NAME} PRIVATE dl)
target_link_libraries(${PROJECT_NAME} PRIVATE hebench_common-lib)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
