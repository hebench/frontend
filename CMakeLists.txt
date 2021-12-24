cmake_minimum_required(VERSION 3.13)
project(frontend LANGUAGES C CXX)
set(CMAKE_PROJECT_NAME ${PROJECT_NAME})

if(CMAKE_BUILD_TYPE)
  set(RELEASE_TYPES
      Debug
      Release
      RelWithDebInfo
      MinSizeRel)
  list(FIND RELEASE_TYPES ${CMAKE_BUILD_TYPE} INDEX_FOUND)
  if(${INDEX_FOUND} EQUAL -1)
    message(
      FATAL_ERROR
        "CMAKE_BUILD_TYPE must be one of Debug, Release, RelWithDebInfo, or MinSizeRel"
      )
  endif()
else()
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of Build" FORCE)
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# versioning
configure_file("VERSION" "VERSION")
file(STRINGS "VERSION" ${CMAKE_PROJECT_NAME}_VERSION)
list(LENGTH ${CMAKE_PROJECT_NAME}_VERSION ${CMAKE_PROJECT_NAME}_VERSION_LEN)
if(${CMAKE_PROJECT_NAME}_VERSION_LEN LESS 4)
    message(FATAL_ERROR "Invalid project version file format.")
endif()
list(GET ${CMAKE_PROJECT_NAME}_VERSION 0 ${CMAKE_PROJECT_NAME}_VERSION_PREFIX)
if(NOT ${CMAKE_PROJECT_NAME}_VERSION_PREFIX STREQUAL "v")
    message(FATAL_ERROR "Invalid project version prefix format read from file.")
endif()
list(GET ${CMAKE_PROJECT_NAME}_VERSION 1 ${CMAKE_PROJECT_NAME}_VERSION_MAJOR)
list(GET ${CMAKE_PROJECT_NAME}_VERSION 2 ${CMAKE_PROJECT_NAME}_VERSION_MINOR)
list(GET ${CMAKE_PROJECT_NAME}_VERSION 3 ${CMAKE_PROJECT_NAME}_VERSION_REVISION)
set(VERSION_TXT "v${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}.${${CMAKE_PROJECT_NAME}_VERSION_MINOR}.${${CMAKE_PROJECT_NAME}_VERSION_REVISION}") 
if(${CMAKE_PROJECT_NAME}_VERSION_LEN GREATER 4)
    list(GET ${CMAKE_PROJECT_NAME}_VERSION 4 ${CMAKE_PROJECT_NAME}_VERSION_BUILD)
    set(VERSION_TXT "${VERSION_TXT}-${${CMAKE_PROJECT_NAME}_VERSION_BUILD}")
else()
    set(${CMAKE_PROJECT_NAME}_VERSION_BUILD "")
endif()
message(STATUS "Project Version: ${VERSION_TXT}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

cmake_policy(SET CMP0079 NEW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

option(HIDE_EXT_WARNINGS "Attempts to hide all warnings encountered by third-party projects" OFF)
message(STATUS "HIDE_EXT_WARNINGS: ${HIDE_EXT_WARNINGS}")

include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

message(STATUS "CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

# third party
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/third-party)

# subprojects
add_subdirectory(common-lib)
add_subdirectory(report_gen)
add_subdirectory(dynamic_lib_load)
add_subdirectory(dataloader)

## APIBridge
set(_COMPONENT_NAME "API_BRIDGE")
set(_COMPONENT_HEADER "hebench/api_bridge/api.h")
set(_COMPONENT_LIB_NAME "hebench_cpp")
set(_HEADER_ONLY FALSE)
set(_LINK_TO "hebench_dynamic_lib_load")
include(cmake/utils/import-library.cmake)
include(cmake/third-party/${_COMPONENT_NAME}_check.cmake)

add_subdirectory(test_harness)

## yaml-cpp
set(_COMPONENT_NAME "YAML_CPP")
set(_COMPONENT_HEADER "yaml-cpp/yaml.h")
set(_COMPONENT_LIB_NAME "yaml-cpp")
set(_HEADER_ONLY FALSE)
set(_LINK_TO "test_harness")
include(cmake/utils/import-library.cmake)
