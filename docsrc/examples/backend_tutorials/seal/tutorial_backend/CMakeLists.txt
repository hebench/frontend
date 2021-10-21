cmake_minimum_required(VERSION 3.12)
project(be_tutorial LANGUAGES C CXX)

# C++ version (SEAL requires 17)
set(CMAKE_CXX_STANDARD 17) # C++ standard C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tutorial_engine_seal.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tutorial_eltwiseadd_benchmark_seal.cpp"
    )
set(${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_error_seal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_engine_seal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_eltwiseadd_benchmark_seal.h"
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

# Microsoft SEAL (version 3.6)
set(SEAL_INCLUDE_DIR "/usr/local/include/SEAL-3.6")
target_include_directories(${PROJECT_NAME} PRIVATE ${SEAL_INCLUDE_DIR}) # point to include for SEAL
find_library(SEAL_FOUND NAMES libseal-3.6.a HINTS "/usr/local/lib")
if(SEAL_FOUND)
    add_library(seal-3.6 UNKNOWN IMPORTED)
    # populate the found library with its properties
    set_property(TARGET seal-3.6 PROPERTY IMPORTED_LOCATION ${SEAL_FOUND})
    set_property(TARGET seal-3.6 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SEAL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "libseal-3.6.a not found.")
endif()

# link found library
target_link_libraries(${PROJECT_NAME} PUBLIC "-Wl,--whole-archive" seal-3.6 "-Wl,--no-whole-archive")

# extra compile options
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra) # show warnings

# install options
include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
install(TARGETS ${PROJECT_NAME} DESTINATION lib)

