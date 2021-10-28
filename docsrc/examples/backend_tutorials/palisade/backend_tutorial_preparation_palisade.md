Quickstart C++ Wrapper Backend Tutorial - Preparation {#be_tutorial_preparation_palisade}
========================
## Steps
### 1. Copy example 
Start by copying api_bridge_example_be to a new folder with the name of the desired backend

`api_bridge_example_backend` -> `tutorial_backend`
	
### 2. Rename files to new backend name 
After copying, the next step is to rename all of the included files to match the scheme of the new backend name. The files to rename are:

- `src/ex_engine_palisade.cpp` -> `src/tutorial_engine_palisade.cpp`
- `src/ex_benchmark_palisade.cpp` -> `src/tutorial_eltwiseadd_benchmark_palisade.cpp`
- `include/ex_benchmark_palisade.h` -> `include/tutorial_eltwiseadd_benchmark_palisade.h`
- `include/ex_engine_palisade.h` -> `include/tutorial_engine_palisade.h`
- new file -> `include/tutorial_error_palisade.h`
	
### 3. Update build system
Update the `CMakeLists.txt` to become stand-alone: point to the new file names, change the target name as appropriate, locate include directory for and link hebench_cpp static library, locate and link other third-party dependencies, like PALISADE for this tutorial.

For simplicity, this build system will assume that all dependencies have been pre-compiled (PALISADE is assumed to be installed). Creating a robust build system is beyond the scope of this tutorial.

Find the listing of the full `CMakeLists.txt` stand-alone below. Make sure to replace `"/include/directory/for/api_bridge"` and `"/directory/containing/libhebench_cpp.a"` for the correct paths.

```cmake
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
```
	
### 4. Update includes in source files
After renaming our files for our new engine we must update the includes in the example files as follows. 

**tutorial_engine_palisade.cpp**

```cpp
#include "../include/ex_engine_palisade.h"
#include <cstring>

// include all benchmarks
#include "../include/ex_benchmark_palisade.h"
```
changed to 

```cpp
#include <cstring>
#include "tutorial_engine_palisade.h"
#include "tutorial_error_palisade.h"

// include all benchmarks
#include "tutorial_eltwiseadd_benchmark_palisade.h"
```
	
**tutorial_eltwiseadd_benchmark_palisade.cpp**

```cpp
#include "../include/ex_benchmark_palisade.h"
#include "../include/ex_engine_palisade.h"
```
	
changed to

```cpp
#include "tutorial_eltwiseadd_benchmark_palisade.h"
#include "tutorial_engine_palisade.h"
#include "tutorial_error_palisade.h"
```

### 5. Define any new error codes
Add the following content to `tutorial_error_palisade.h`:

```cpp
#pragma once
#define TUTORIAL_ECODE_PALISADE_ERROR 2
```
	
### 6. Refactor classes
Now that we have renamed the files to a name suitable for our new backend we must rename the example classes described in the files to the names of our new backend. For this step it is highly recommend to use an IDE with support for automatic class refactoring.

Refactor the following classes to the new name:

**tutorial_engine_palisade.h**

- `ExampleEngine` -> `TutorialEngine`
	
**tutorial_eltwiseadd_benchmark_palisade.h**

- `ExampleBenchmark` -> `TutorialEltwiseAddBenchmark`
- `ExampleBenchmarkDescription` -> `TutorialEltwiseAddBenchmarkDescription`

<hr/>
#### Checkpoint
After completing this step it may be a good idea to try compiling the backend and running it. In its current state, if the above steps have been followed, the backend should run and perform a simple matrix multiplication in plain text. The next steps will discuss the steps to perform our benchmark instead.
<hr/>

## Tutorial steps

[Tutorial Home](backend_tutorial_palisade.md)<br/>
<b>Preparation</b><br/>
[Engine Initialization and Benchmark Description](backend_tutorial_init_palisade.md)<br/>
[Benchmark Implementation](backend_tutorial_impl_palisade.md)<br/>
[File References](backend_tutorial_files_palisade.md)
