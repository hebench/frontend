set(CONTENT_NAME yaml-cpp)

set(CMAKE_CXX_FLAGS_CURRENT "${CMAKE_CXX_FLAGS}")

FetchContent_Declare(
  ${CONTENT_NAME}
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp
  GIT_TAG        ${${_COMPONENT_NAME}_TAG}
  SUBBUILD_DIR   ${FETCHCONTENT_BASE_DIR}/${CONTENT_NAME}/${CONTENT_NAME}-subbuild
  SOURCE_DIR     ${FETCHCONTENT_BASE_DIR}/${CONTENT_NAME}/${CONTENT_NAME}-src
  BINARY_DIR     ${FETCHCONTENT_BASE_DIR}/${CONTENT_NAME}/${CONTENT_NAME}-build
)

FetchContent_GetProperties(${CONTENT_NAME})
if(NOT ${CONTENT_NAME}_POPULATED)
  FetchContent_Populate(${CONTENT_NAME})
  set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Build/Install Google Test for Yaml-CPP unit tests")
  set(YAML_CPP_INSTALL OFF CACHE BOOL "Disable generation of install target")
  if (${HIDE_EXT_WARNINGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
  endif()
  add_subdirectory(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_CURRENT}")
endif()

