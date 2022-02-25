Generic Workload Example {#generic_wl_example}
========================

[TOC]

The *Generic Workload* is intended as a shortcut to help test and benchmark operations that are not directly supported by HEBench. See @ref generic_workload for the full definition and reference. The nomenclature defined there is the same one used for the rest of this document.

Here is included an example on how to create a backend to benchmark a custom workload, and how to prepare configuration and dataset files for it.

## Problem Statement

Let's assume that the function we want to benchmark maps `(InputParam[0], InputParam[1])` to `(ResultComponent[0], ResultComponent[1], ResultComponent[2])` where `InputParam[0]` is a vector with two components, `InputParam[1]` is a vector with two components as well and:

```cpp
ResultComponent[0] = InputParam[0] + InputParam[1]
ResultComponent[1] = InputParam[0] - InputParam[1]
ResultComponent[2] = InputParam[0] . InputParam[1] // dot product
```

Such operation is not supported by the standard workloads defined by HEBench, so, we need a generic workload to measure the performance of our backend implementation.

Let's also assume that we want to test in *offline* category using a dataset with `2` samples for `InputParam[0]` and `5` samples for `InputParam[1]`, which makes for `2 x 5 = 10` possible combinations of input samples.

## Example Configuration

These are the values that would be set in the [benchmark yaml configuration file](@ref config_file_reference).

### Configuration for input InputParam[0]
- Number of samples: `2`


### Configuration for input InputParam[1]
- Number of samples: `5`

### Workload parameters

| Index | Name | Type | Value |
|-|-|-|-|
| `0` | `n` | `uint64_t` | `2` |
| `1` | `m` | `uint64_t` | `3` |
| `2` | `length_InputParam0` | `uint64_t` | `2` |
| `3` | `length_InputParam1` | `uint64_t` | `2` |
| `4` | `length_ResultComponent0` | `uint64_t` | `2` |
| `5` | `length_ResultComponent1` | `uint64_t` | `2` |
| `6` | `length_ResultComponent2` | `uint64_t` | `1` |

### Benchmark Configuration File

The configuration given above translates to the following configuration file:

[config.yaml](@ref generic_wl_config_yaml)

## Example Dataset

For validation purposes, we must provide a dataset with our inputs and the expected output of the operation function evaluated on each input sample. The dataset must be provided in one of the dataset formats as supported by the [dataset loader](@ref dataset_loader_overview).

### Samples for InputParam[0]

| Sample | Value |
|-|-|
| 0 |`(1, 2)`|
| 1 |`(-1, -1)`|

### Samples for InputParam[1]

| Sample | Value |
|-|-|
| 0 |`(-1, 0)`|
| 1 |`(1, 1)`|
| 2 |`(2, -1)`|
| 2 |`(-2, 1)`|
| 2 |`(1, -1)`|

### Full dataset as must be provided to Test Harness

Note in the following table that the order of combinations for input samples to Test Harness is defined to be row major; this is, the index for the most significant operation parameter moves faster when computing the position of an input sample in the ordering. For full definition see @ref results_order .

| Sample | Inputs | | Outputs | | |
|-|-|-|-|-|-|
|     | `InputParam[0]`   | `InputParam[1]`    | `ResultComponent[0]`   | `ResultComponent[1]`   | `ResultComponent[2]` |
| `0` | `(1, 2)` | `(-1, 0)` | `(0, 2)` | `(2, 2)` | `(-1)`|
| `1` | `(1, 2)` | `(1, 1)` | `(2, 3)` | `(0, 1)` | `(3)`|
| `2` | `(1, 2)` | `(2, -1)` | `(3, 1)` | `(-1, 3)` | `(0)`|
| `3` | `(1, 2)` | `(-2, 1)` | `(-1, 3)` | `(3, 1)` | `(0)`|
| `4` | `(1, 2)` | `(1, -1)` | `(2, 1)` | `(0, 3)` | `(-1)`|
| `5` | `(-1, -1)` | `(-1, 0)` | `(-2, -1)` | `(0, -1)` | `(1)`|
| `6` | `(-1, -1)` | `(1, 1)` | `(0, 0)` | `(-2, -2)` | `(-2)`|
| `7` | `(-1, -1)` | `(2, -1)` | `(1, -2)` | `(-3, 0)` | `(-1)`|
| `8` | `(-1, -1)` | `(-2, 1)` | `(-3, 0)` | `(1, -2)` | `(1)`|
| `9` | `(-1, -1)` | `(1, -1)` | `(0, -2)` | `(-2, 0)` | `(0)`|

This dataset can be formatted into a CSV compatible with Test Harness's external dataset loader as follows:

[dataset.csv](@ref generic_wl_dataset_csv)

## Example Backend

These files implement an example backend that performs our custom operation in clear text using a Generic Workload as explained.

  Example File   | Description
-------------- | ------------
[config.yaml](@ref generic_wl_config_yaml)      | Generic workload example backend benchmark configuration.
[dataset.csv](@ref generic_wl_dataset_csv)      | Generic workload example dataset in CSV.
[CMakeLists.txt](@ref generic_wl_cmakelists)      | Generic workload example backend CMakeLists.
@ref generic_wl_engine.h      | Generic workload engine header.
@ref generic_wl_engine.cpp    | Generic workload engine implementation.
@ref generic_wl_benchmark.h   | Generic workload backend benchmark header.
@ref generic_wl_benchmark.cpp | Generic workload backend benchmark implementation.

