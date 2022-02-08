# Copyright (C) 2021 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

project(test_harness)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/include/hebench_version.h.in1")
configure_file("${CMAKE_CURRENT_BINARY_DIR}/include/hebench_version.h.in1" "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_version.h")

# Benchmark General Files
set(${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/datagen_helper/include/datagen_helper.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/categories/include/hebench_benchmark_category.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/categories/include/hebench_benchmark_latency.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/categories/include/hebench_benchmark_offline.h"
    )

set(${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/datagen_helper/src/datagen_helper.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/categories/src/hebench_benchmark_category.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/categories/src/hebench_benchmark_latency.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/categories/src/hebench_benchmark_offline.cpp"
    )

# Logisitc Regression Inference
list(APPEND ${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/LogisticRegression/include/hebench_logreg.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/LogisticRegression/latency/include/hebench_logreg_l.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/LogisticRegression/offline/include/hebench_logreg_o.h"
    )

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/LogisticRegression/src/hebench_logreg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/LogisticRegression/latency/src/hebench_logreg_l.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/LogisticRegression/offline/src/hebench_logreg_o.cpp"
    )

# DotProduct
list(APPEND ${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/DotProduct/include/hebench_dotproduct.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/DotProduct/latency/include/hebench_dotproduct_l.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/DotProduct/offline/include/hebench_dotproduct_o.h"
    )

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/DotProduct/src/hebench_dotproduct.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/DotProduct/latency/src/hebench_dotproduct_l.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/DotProduct/offline/src/hebench_dotproduct_o.cpp"
    )

# EltwiseMult
list(APPEND ${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseMult/include/hebench_eltwisemult.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseMult/latency/include/hebench_eltwisemult_l.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseMult/offline/include/hebench_eltwisemult_o.h"
    )

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseMult/src/hebench_eltwisemult.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseMult/latency/src/hebench_eltwisemult_l.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseMult/offline/src/hebench_eltwisemult_o.cpp"
    )

# EltwiseAdd
list(APPEND ${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseAdd/include/hebench_eltwiseadd.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseAdd/latency/include/hebench_eltwiseadd_l.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseAdd/offline/include/hebench_eltwiseadd_o.h"
    )

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseAdd/src/hebench_eltwiseadd.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseAdd/latency/src/hebench_eltwiseadd_l.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/EltwiseAdd/offline/src/hebench_eltwiseadd_o.cpp"
    )

# MatMult
list(APPEND ${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/MatrixMultiply/include/hebench_matmult.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/MatrixMultiply/latency/include/hebench_matmult_l.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/MatrixMultiply/offline/include/hebench_matmult_o.h"
    )

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/MatrixMultiply/src/hebench_matmult.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/MatrixMultiply/latency/src/hebench_matmult_l.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/MatrixMultiply/offline/src/hebench_matmult_o.cpp"
    )

# Generic Workload
list(APPEND ${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/Generic/include/hebench_genericwl.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/Generic/latency/include/hebench_genericwl_l.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/Generic/offline/include/hebench_genericwl_o.h"
    )

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/Generic/src/hebench_genericwl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/Generic/latency/src/hebench_genericwl_l.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/Generic/offline/src/hebench_genericwl_o.cpp"
    )

# main application
list(APPEND ${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_benchmark_factory.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_benchmark_description.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_config.h"
    #"${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_dataset_loader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_engine.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_ibenchmark.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_idata_loader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_math_utils.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_types_harness.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_utilities.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hebench_version.h"
    )

list(APPEND ${PROJECT_NAME}_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/inl/hebench_utilities.inl"
    )

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_benchmark_factory.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_config.cpp"
    #"${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_dataset_loader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_engine.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_ibenchmark.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_idata_loader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_math_utils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hebench_utilities.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    )

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# API Bridge
#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/api_bridge)

# add lib-common-lib.a dependency
target_link_libraries(${PROJECT_NAME} PRIVATE hebench_common-lib)

# dynamic_lib_load
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../dynamic_lib_load/include)
target_link_libraries(${PROJECT_NAME} PRIVATE hebench_dynamic_lib_load)

# dataset_loader
target_link_libraries(${PROJECT_NAME} PRIVATE hebench_dataset_loader)

# report generator
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../report_gen/include)
target_link_libraries(${PROJECT_NAME} PRIVATE hebench_reportgen_lib)
target_link_libraries(${PROJECT_NAME} PRIVATE hebench_reportgen)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
