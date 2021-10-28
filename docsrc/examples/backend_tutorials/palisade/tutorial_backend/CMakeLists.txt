cmake_minimum_required(VERSION 3.12)
project(be_tutorial LANGUAGES C CXX)

# C++ version
set(CMAKE_CXX_STANDARD 17) # C++ standard C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tutorial_engine_palisade.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tutorial_eltwiseadd_benchmark_palisade.cpp"
    )
set(${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_error_palisade.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_engine_palisade.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_eltwiseadd_benchmark_palisade.h"
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
    message(STATUS "libhebench_cpp.a found")
else()
    message(FATAL_ERROR "libhebench_cpp.a not found.")
endif()

# link found library
target_link_libraries(${PROJECT_NAME} PUBLIC "-Wl,--whole-archive" hebench_cpp "-Wl,--no-whole-archive")

# find other third party dependencies

# PALISADE (version 1.11.3)
set(PALISADE_HINT_DIR "/usr/local/")
include(ExternalProject)

option(PALISADE_PREBUILT "" ON) # Set to ON/OFF to use prebuilt installation
message(STATUS "PALISADE_PREBUILT: ${PALISADE_PREBUILT}")

if (PALISADE_PREBUILT) # Skip download from gitlab

  message(STATUS "PALISADE_HINT_DIR: ${PALISADE_HINT_DIR}")
  find_package(Palisade
    HINTS ${PALISADE_HINT_DIR}
    REQUIRED)

  message(STATUS "PALISADE_INCLUDE ${PALISADE_INCLUDE}")
  message(STATUS "PALISADE_LIBRARIES ${PALISADE_LIBRARIES}")
  message(STATUS "PALISADE_LIBDIR ${PALISADE_LIBDIR}")
  message(STATUS "OPENMP_INCLUDES ${OPENMP_INCLUDES}")
  message(STATUS "OPENMP_LIBRARIES ${OPENMP_LIBRARIES}")

  add_library(libpalisade INTERFACE)
  target_include_directories(libpalisade INTERFACE
    ${OPENMP_INCLUDES} # Will be empty if PALISADE is built single-threaded
    ${PALISADE_INCLUDE}
    ${PALISADE_INCLUDE}/third-party/include
    ${PALISADE_INCLUDE}/core
    ${PALISADE_INCLUDE}/pke
    ${PALISADE_INCLUDE}/binfhe
    )

  target_link_directories(libpalisade INTERFACE ${PALISADE_LIBDIR})
  target_link_libraries(libpalisade INTERFACE ${PALISADE_LIBRARIES} ${OPENMP_LIBRARIES})

  # Ignore errors from PALISADE
  add_compile_options(-Wno-error=unused-parameter -Wno-error=ignored-qualifiers -Wno-error=deprecated-copy)

else()

  set(PALISADE_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ext_palisade)
  set(PALISADE_SRC_DIR ${PALISADE_PREFIX}/src/ext_palisade/)
  set(PALISADE_REPO_URL https://gitlab.com/palisade/palisade-release.git)
  set(PALISADE_GIT_TAG v1.11.5)

  if (ENABLE_INTEL_HEXL)
    ExternalProject_Add(
      ext_palisade
      GIT_REPOSITORY ${PALISADE_REPO_URL}
      GIT_TAG ${PALISADE_GIT_TAG}
      PREFIX ${PALISADE_PREFIX}
      CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=${PALISADE_PREFIX}
      -DCMAKE_CXX_FLAGS=${SINGLE_THREAD_CMAKE_CXX_FLAGS}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_INSTALL_LIBDIR=${PALISADE_PREFIX}/${CMAKE_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_INCLUDEDIR=${PALISADE_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}
      -DBUILD_UNITTESTS=OFF
      -DBUILD_EXAMPLES=OFF
      -DBUILD_BENCHMARKS=ON
      -DBUILD_EXTRAS=OFF
      -DBUILD_STATIC=OFF
      -DWITH_BE2=ON
      -DWITH_BE4=OFF
      -DWITH_TCM=OFF
      -DWITH_OPENMP=OFF
      -DWITH_NATIVEOPT=ON
      -DWITH_INTEL_HEXL=${ENABLE_INTEL_HEXL}
      -DINTEL_HEXL_PREBUILT=OFF
      -DINTEL_HEXL_HINT_DIR=${INTEL_HEXL_HINT_DIR}
      BUILD_COMMAND $(MAKE) -j all
      # Skip updates
      UPDATE_COMMAND ""
    )
  else()
    ExternalProject_Add(
      ext_palisade
      GIT_REPOSITORY ${PALISADE_REPO_URL}
      GIT_TAG ${PALISADE_GIT_TAG}
      PREFIX ${PALISADE_PREFIX}
      CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=${PALISADE_PREFIX}
      -DCMAKE_CXX_FLAGS=${SINGLE_THREAD_CMAKE_CXX_FLAGS}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_INSTALL_LIBDIR=${PALISADE_PREFIX}/${CMAKE_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_INCLUDEDIR=${PALISADE_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}
      -DBUILD_UNITTESTS=OFF
      -DBUILD_EXAMPLES=OFF
      -DBUILD_BENCHMARKS=ON
      -DBUILD_EXTRAS=OFF
      -DBUILD_STATIC=OFF
      -DWITH_BE2=ON
      -DWITH_BE4=OFF
      -DWITH_TCM=OFF
      -DWITH_OPENMP=OFF
      -DWITH_NATIVEOPT=ON
      -DWITH_INTEL_HEXL=OFF
      BUILD_COMMAND $(MAKE) -j all
      # Skip updates
      UPDATE_COMMAND ""
    )
  endif()

  ExternalProject_Get_Property(ext_palisade SOURCE_DIR BINARY_DIR)

  # PALISADE core
  add_library(libpalisade_core INTERFACE)
  add_dependencies(libpalisade_core ext_palisade)
  target_include_directories(libpalisade_core SYSTEM
                            INTERFACE ${PALISADE_SRC_DIR}/src/core/include)
  target_include_directories(libpalisade_core SYSTEM
                            INTERFACE ${BINARY_DIR}/src/core)
  target_include_directories(libpalisade_core SYSTEM
                            INTERFACE ${SOURCE_DIR}/third-party/cereal/include)
  target_link_libraries(libpalisade_core
                        INTERFACE ${BINARY_DIR}/lib/libPALISADEcore.so)

  # PALISADE pke
  add_library(libpalisade_pke INTERFACE)
  add_dependencies(libpalisade_pke ext_palisade libpalisade_core)

  target_include_directories(libpalisade_pke SYSTEM
                            INTERFACE ${PALISADE_SRC_DIR}/src/pke/include)
  target_link_libraries(libpalisade_pke INTERFACE libpalisade_core)
  target_link_libraries(libpalisade_pke
                        INTERFACE ${BINARY_DIR}/lib/libPALISADEpke.so)

  # Create interface to multiple PALISADE libraries to be consistent with PALISADE_PREBUILT=ON
  add_library(libpalisade INTERFACE)
  target_link_libraries(libpalisade INTERFACE
    libpalisade_core  libpalisade_pke
    # libpalisade_binfhe
    )

endif()

# link found library
target_link_libraries(${PROJECT_NAME} PUBLIC libpalisade)

# extra compile options
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra) # show warnings

# install options
include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
install(TARGETS ${PROJECT_NAME} DESTINATION lib)

