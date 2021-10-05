Quickstart C++ Wrapper Backend Tutorial - Preparation {#be_tutorial_preparation}
========================
## Steps
### 1. Copy example 
Start by copying api_bridge_example_be to a new folder with the name of the desired backend

`api_bridge_example_backend` -> `tutorial_backend`
	
### 2. Rename files to new backend name 
After copying, the next step is to rename all of the included files to match the scheme of the new backend name. The files to rename are:

- `src/ex_engine.cpp` -> `src/tutorial_engine.cpp`
- `src/ex_benchmark.cpp` -> `src/tutorial_eltwiseadd_benchmark.cpp`
- `include/ex_benchmark.h` -> `include/tutorial_eltwiseadd_benchmark.h`
- `include/ex_engine.h` -> `include/tutorial_engine.h`
- new file -> `include/tutorial_error.h`
	
### 3. Update build system
Update the `CMakeLists.txt` to become stand-alone: point to the new file names, change the target name as appropriate, locate include directory for and link hebench_cpp static library, locate and link other third-party dependencies, like Microsoft SEAL for this tutorial.

For simplicity, this build system will assume that all dependencies have been pre-compiled (Microsoft SEAL is assumed to be installed). Creating a robust build system is beyond the scope of this tutorial.

Find the listing of the full `CMakeLists.txt` stand-alone below. Make sure to replace `"/include/directory/for/api_bridge"` and `"/directory/containing/libhebench_cpp.a"` for the correct paths.

```cmake
cmake_minimum_required(VERSION 3.12)
project(be_tutorial LANGUAGES C CXX)

# C++ version (SEAL requires 17)
set(CMAKE_CXX_STANDARD 17) # C++ standard C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tutorial_engine.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/tutorial_eltwiseadd_benchmark.cpp"
    )
set(${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_error.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_engine.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/tutorial_eltwiseadd_benchmark.h"
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

# Microsoft SEAL (version 3.5)
set(SEAL_INCLUDE_DIR "/usr/local/include/SEAL-3.5")
target_include_directories(${PROJECT_NAME} PRIVATE ${SEAL_INCLUDE_DIR}) # point to include for SEAL
find_library(SEAL_FOUND NAMES libseal-3.5.a HINTS "/usr/local/lib")
if(SEAL_FOUND)
    add_library(seal-3.5 UNKNOWN IMPORTED)
    # populate the found library with its properties
    set_property(TARGET seal-3.5 PROPERTY IMPORTED_LOCATION ${SEAL_FOUND})
    set_property(TARGET seal-3.5 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SEAL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "libseal-3.5.a not found.")
endif()

# link found library
target_link_libraries(${PROJECT_NAME} PUBLIC "-Wl,--whole-archive" seal-3.5 "-Wl,--no-whole-archive")

# extra compile options
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra) # show warnings

# install options
include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
install(TARGETS ${PROJECT_NAME} DESTINATION lib)

```
	
### 4. Update includes in source files
After renaming our files for our new engine we must update the includes in the example files as follows. 

**tutorial_engine.cpp**

```cpp
#include "../include/ex_engine.h"
#include <cstring>

// include all benchmarks
#include "../include/ex_benchmark.h"
```
changed to 

```cpp
#include <cstring>
#include "tutorial_engine.h"
#include "tutorial_error.h"

// include all benchmarks
#include "tutorial_eltwiseadd_benchmark.h"
```
	
**tutorial_eltwiseadd_benchmark.cpp**

```cpp
#include "../include/ex_benchmark.h"
#include "../include/ex_engine.h"
```
	
changed to

```cpp
#include "tutorial_eltwiseadd_benchmark.h"
#include "tutorial_engine.h"
#include "tutorial_error.h"
```

### 5. Define any new error codes
Add the following content to `tutorial_error.h`:

```cpp
#pragma once
#define TUTORIAL_ECODE_SEAL_ERROR 2
```
	
### 6. Refactor classes
Now that we have renamed the files to a name suitable for our new backend we must rename the example classes described in the files to the names of our new backend. For this step it is highly recommend to use an IDE with support for automatic class refactoring.

Refactor the following classes to the new name:

**tutorial_engine.h**

- `ExampleEngine` -> `TutorialEngine`
	
**tutorial_eltwiseadd_benchmark.h**

- `ExampleBenchmark` -> `TutorialEltwiseAddBenchmark`
- `ExampleBenchmarkDescription` -> `TutorialEltwiseAddBenchmarkDescription`

<hr/>
#### Checkpoint
After completing this step it may be a good idea to try compiling the backend and running it. In its current state, if the above steps have been followed, the backend should run and perform a simple matrix multiplication in plain text. The next steps will discuss the steps to perform our benchmark instead.
<hr/>

## Tutorial steps

[Tutorial Home](backend_tutorial.md)<br/>
<b>Preparation</b><br/>
[Engine Initialization and Benchmark Description](backend_tutorial_init.md)<br/>
[Benchmark Implementation](backend_tutorial_impl.md)<br/>
[File References](backend_tutorial_files.md)
