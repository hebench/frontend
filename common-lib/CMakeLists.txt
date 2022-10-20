# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0


# libcommon-lib.a - Static library containing common utilities for C++
#   applications such as command line argument parser, file system,
#   threading, etc.
#

cmake_minimum_required(VERSION 3.12)

project(hebench_common-lib VERSION 0.1.5 LANGUAGES C CXX)
set(ACTIVE_TARGET ${PROJECT_NAME})

set(HEADERS
    modules/general/include/pool.h
    modules/general/include/nocopy.h
    modules/general/include/error.h
    modules/general/include/memory_buffer.h
    modules/general/include/api_decl.h
    modules/general/include/hebench_utilities.h
    modules/general/include/hebench_math_utils.h
    modules/args_parser/include/args_parser.h
    modules/config_reader/include/config_reader.h
    modules/logging/include/logging.h
    modules/threading/include/threading.h
    modules/threading/include/safe_queue.h
    modules/timer/include/timer.h
    )

list(APPEND HEADERS
    modules/threading/include/inl/safe_queue.inl
    )

set(SOURCES
    modules/args_parser/src/args_parser.cpp
    modules/config_reader/src/config_reader.cpp
    modules/threading/src/threading.cpp
    modules/general/src/hebench_utilities.cpp
    modules/general/src/hebench_math_utils.cpp
    modules/general/src/memory_buffer.cpp
    modules/logging/src/logging.cpp
    )

# create the target library
add_library(
    ${ACTIVE_TARGET} STATIC
    # files
    ${HEADERS}
    ${SOURCES}
    )

# add the include path to this library
target_include_directories(${ACTIVE_TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# set compiler properties
set_target_properties(${ACTIVE_TARGET} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${ACTIVE_TARGET} Threads::Threads)
target_compile_options(${ACTIVE_TARGET} PRIVATE -Wall -Wextra -fPIC)
