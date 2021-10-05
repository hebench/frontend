# Copyright (C) 2020 Intel Corporation


# libcommon-lib.a - Static library containing common utilities for C++
#   applications such as command line argument parser, file system,
#   threading, etc.
#

cmake_minimum_required(VERSION 3.12)

project(common-lib VERSION 0.1.4 LANGUAGES C CXX)
set(ACTIVE_TARGET hebench_common-lib)

set(HEADERS
    modules/general/include/pool.h
    modules/general/include/nocopy.h
    modules/general/include/error.h
    modules/general/include/memory_buffer.h
    modules/general/include/api_decl.h
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

target_link_libraries(hebench_common-lib Threads::Threads)
target_compile_options(hebench_common-lib PRIVATE -Wall -Wextra)
