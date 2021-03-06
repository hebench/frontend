set(_${_COMPONENT_NAME}_INCLUDE_DIR "/usr/local/include")
set(_${_COMPONENT_NAME}_LIB_DIR "/usr/local/lib")
set(_USER_DEFINED_PATHS FALSE)

message(STATUS "===== Configuring Third-Pary: ${_COMPONENT_NAME} =====")

# versioning
configure_file("cmake/third-party/${_COMPONENT_NAME}.version" "cmake/third-party/${_COMPONENT_NAME}.version")
file(STRINGS "cmake/third-party/${_COMPONENT_NAME}.version" ${_COMPONENT_NAME}_REQUIRED_VERSION)
list(LENGTH ${_COMPONENT_NAME}_REQUIRED_VERSION ${_COMPONENT_NAME}_REQUIRED_VERSION_LEN)
if(${_COMPONENT_NAME}_REQUIRED_VERSION_LEN LESS 1)
    message(FATAL_ERROR "Invalid project version file format.")
endif()
list(GET ${_COMPONENT_NAME}_REQUIRED_VERSION 0 ${_COMPONENT_NAME}_REQUIRED_VERSION_PREFIX)
set(${_COMPONENT_NAME}_TAG "main")
if(${_COMPONENT_NAME}_REQUIRED_VERSION_PREFIX STREQUAL "v")
    if(${_COMPONENT_NAME}_REQUIRED_VERSION_LEN LESS 4)
        message(FATAL_ERROR "Invalid project version file format.")
    endif()
    list(GET ${_COMPONENT_NAME}_REQUIRED_VERSION 1 ${_COMPONENT_NAME}_REQUIRED_VERSION_MAJOR)
    list(GET ${_COMPONENT_NAME}_REQUIRED_VERSION 2 ${_COMPONENT_NAME}_REQUIRED_VERSION_MINOR)
    list(GET ${_COMPONENT_NAME}_REQUIRED_VERSION 3 ${_COMPONENT_NAME}_REQUIRED_VERSION_REVISION)
    set(${_COMPONENT_NAME}_TAG "v${${_COMPONENT_NAME}_REQUIRED_VERSION_MAJOR}.${${_COMPONENT_NAME}_REQUIRED_VERSION_MINOR}.${${_COMPONENT_NAME}_REQUIRED_VERSION_REVISION}")
    if(${_COMPONENT_NAME}_REQUIRED_VERSION_LEN GREATER 4)
        list(GET ${_COMPONENT_NAME}_REQUIRED_VERSION 4 ${_COMPONENT_NAME}_REQUIRED_VERSION_BUILD)
        set(${_COMPONENT_NAME}_TAG "${${_COMPONENT_NAME}_TAG}-${${_COMPONENT_NAME}_REQUIRED_VERSION_BUILD}")
    else()
        set(${_COMPONENT_NAME}_REQUIRED_VERSION_BUILD "")
    endif()
    set(${_COMPONENT_NAME}_TAG ${${_COMPONENT_NAME}_TAG})
elseif(${_COMPONENT_NAME}_REQUIRED_VERSION_PREFIX STREQUAL "")
    message(FATAL_ERROR "No version or tag found for component ${_COMPONENT_NAME}")
else()
    list(GET ${_COMPONENT_NAME}_REQUIRED_VERSION 0 ${_COMPONENT_NAME}_TAG)
endif()

message(STATUS "Required Version: ${${_COMPONENT_NAME}_TAG}")

# checking for provided pre-installed
if(DEFINED ${_COMPONENT_NAME}_INSTALL_DIR)
    get_filename_component(_${_COMPONENT_NAME}_INCLUDE_DIR "${${_COMPONENT_NAME}_INSTALL_DIR}/include" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
    get_filename_component(_${_COMPONENT_NAME}_LIB_DIR "${${_COMPONENT_NAME}_INSTALL_DIR}/lib" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
    set(_USER_DEFINED_PATHS TRUE)
endif()
if(DEFINED ${_COMPONENT_NAME}_INCLUDE_DIR)
    get_filename_component(_${_COMPONENT_NAME}_INCLUDE_DIR "${${_COMPONENT_NAME}_INCLUDE_DIR}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
    set(_USER_DEFINED_PATHS TRUE)
endif()
if(DEFINED ${_COMPONENT_NAME}_LIB_DIR)
    get_filename_component(_${_COMPONENT_NAME}_LIB_DIR "${${_COMPONENT_NAME}_LIB_DIR}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
    set(_USER_DEFINED_PATHS TRUE)
endif()

message(STATUS "${_COMPONENT_NAME}_INCLUDE_DIR: ${_${_COMPONENT_NAME}_INCLUDE_DIR}")
if(NOT ${_HEADER_ONLY})
    message(STATUS "${_COMPONENT_NAME}_LIB_DIR: ${_${_COMPONENT_NAME}_LIB_DIR}")
endif()

# TODO: switch to using find_package

# finding pre-installed from provided, or pulling from remote if not
if(${_HEADER_ONLY})
    set(${_COMPONENT_NAME}_LIB_FOUND TRUE)
else()
    find_library(${_COMPONENT_NAME}_LIB_FOUND NAMES ${_COMPONENT_LIB_NAME} lib${_COMPONENT_LIB_NAME} lib${_COMPONENT_LIB_NAME}.a HINTS "${_${_COMPONENT_NAME}_LIB_DIR}")
endif()
if(${_COMPONENT_NAME}_LIB_FOUND AND EXISTS "${_${_COMPONENT_NAME}_INCLUDE_DIR}/${_COMPONENT_HEADER}")
    message(STATUS "FOUND PRE-INSTALLED ${_COMPONENT_NAME}")
    if(${_HEADER_ONLY})
        add_library(${_COMPONENT_LIB_NAME} INTERFACE)
    else()
        add_library(${_COMPONENT_LIB_NAME} UNKNOWN IMPORTED)
    endif()
    if(NOT ${_HEADER_ONLY}) 
        set_property(TARGET ${_COMPONENT_LIB_NAME} PROPERTY IMPORTED_LOCATION "${${_COMPONENT_NAME}_LIB_FOUND}")
    endif()
    set_property(TARGET ${_COMPONENT_LIB_NAME} APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${_${_COMPONENT_NAME}_INCLUDE_DIR}")
elseif(_USER_DEFINED_PATHS)
    message(FATAL_ERROR "FAILED TO FIND PRE-INSTALLED ${_COMPONENT_NAME} AT ABOVE USER-DEFINED PATHS")
else()
    message(STATUS "No user-defined paths (-D${_COMPONENT_NAME}_[INSTALL|INCLUDE|LIB]_DIR) were set")
    message(STATUS "${_COMPONENT_NAME} could not be found at the default location")
    message(STATUS "Downloading and Installing it...")
    include(cmake/third-party/${_COMPONENT_NAME}.cmake)
    get_target_property(${_COMPONENT_NAME}_INCLUDE_DIR ${_COMPONENT_LIB_NAME} INCLUDE_DIRECTORIES)
endif()
if (${_HEADER_ONLY})
    target_include_directories(${_LINK_TO} PUBLIC ${${_COMPONENT_NAME}_INCLUDE_DIR})
else()
    target_include_directories(${_LINK_TO} PUBLIC ${${_COMPONENT_NAME}_INCLUDE_DIR})
    target_link_libraries(${_LINK_TO} PUBLIC "-Wl,--whole-archive" ${_COMPONENT_LIB_NAME} "-Wl,--no-whole-archive")
endif()
