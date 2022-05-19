Quickstart C++ Wrapper Backend Tutorial - Preparation {#be_tutorial_preparation_palisade}
========================

[TOC]

## Steps

Before attempting these steps, you must have a built version of the API Bridge.

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

\verbinclude docsrc/examples/backend_tutorials/palisade/tutorial_backend/CMakeLists.txt 

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
