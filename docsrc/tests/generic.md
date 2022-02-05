
Generic Workload {#generic_workload}
========================
The *Generic Workload* is intended as a shortcut to help test and benchmark operations that are not directly supported by HEBench. Therefore, a generic workload does not have a predefined operation with known input parameters and outputs, as well as no way of pre-computing ground truths from a set of inputs. This reduces the need to manually add unsupported workloads to the list of standard workloads directly into the HEBench source code (which requires a fair amount of complex work, obeying several rules, recompilation, testing, and so on, for the new workload to properly function).

To provide support for a generic workload, a backend must provide custom flexible workload parameters representing the expected input and output shapes. To benchmark the backend's generic workload, a Test Harness user must supply values to these workload parameters through a benchmark configuration file. The actual input data and ground truths must be supplied through an external dataset. See @ref config_file_reference and @ref dataset_loader_overview for details on how to use configuration files and external datasets.

The following describes the settings required to define a generic workload. See an example at the end of this document for a case that shows how a generic workload could be used.

## Navigation

- [Operation Description](#operation-description)
  - [Operation Parameters](#operation-parameters)
  - [Result](#operation-result)
  - [Details](#operation-details)
- [Workloads](#workloads)
  - [Workload Parameters](#workload-parameters)
- [Categories](#categories)
  - [Category Parameters](#category-parameters)
- [Data Type](#data-type)
- [Data Layout](#data-layout)
- [Dataset](#dataset)
- [Example](#example)

## Operation Description <a name="operation-description"></a>

### Summary
This operation is defined as:

```cpp
(ResultComponent[0], ResultComponent[1], ..., ResultComponent[m - 1]) = op(InputParam[0], InputParam[1], ..., InputParam[n - 1])
```

for some unknown bijective function `op` with `n` input parameters and an output with `m` components. Where `0 < n < HEBENCH_MAX_OP_PARAMS` and `m > 0`.

### Operation Parameters <a name="operation-parameters"></a>

Input: `n` parameters

`InputParam` is an ordered collection of `n` vectors.

| Parameter | Description |
|-|-|
| `0 <= i < n` | `InputParam[i]` is a vector of scalars with an undetermined non-zero number of components. |

The number of components in `InputParam[i]` does not need to be the same as the number of components in `InputParam[j]` for `i != j`.

### Result <a name="operation-result"></a>

Output: `m` components

`ResultComponent` is an ordered collection of `m` vectors.

| Output | Description |
|-|-|
| `0 <= i < m` | `ResultComponent[i]` is a vector of scalars with an undetermined non-zero number of components. |

The number of components in `ResultComponent[i]` does not need to be the same as the number of components in `ResultComponent[j]` for `i != j`.

### Details <a name="operation-details"></a>

The operation for a generic workload is a function described by a configuration file and a given external dataset. The configuration file describes the number of inputs `n`, the number of outputs `m`, the number of components for each input `InputParam[i]`, and the number of components for each output `ResultComponent[j]`. The external dataset provides the mapping of values for all combinations in `InputParam` into all values in `ResultComponent`. See @ref config_file_reference and @ref dataset_loader_overview for details on how to use configuration files and external datasets.

## Workloads <a name="workloads"></a>

This document applies to the following workloads:

```cpp
// From hebench/api_bridge/types.h

hebench:APIBridge::Workload::Generic
```

### Workload Parameters <a name="workload-parameters"></a>

Required workload parameters: `2 + m + n`

| Index | Name | Type | Description |
|-|-|-|-|
| `0` | `n` | `uint64_t` | Number of inputs to the operation (number of vectors in collection `A`). |
| `1` | `m` | `uint64_t` | Number of outputs to the operation (number of vectors in collection `B`). |
| `2` | `length_InputParam0` | `uint64_t` | Number of components in vector `InputParam[0]`. |
| ... |  |  |  |
| `n + 1` | `length_InputParam`*n-1* | `uint64_t` | Number of components in vector `InputParam[n - 1]`. |
| `n + 2` | `length_ResultComponent0` | `uint64_t` | Number of components in vector `ResultComponent[0]`. |
| ... |  |  |  |
| `m + n + 1` | `length_ResultComponent`*m-1* | `uint64_t` | Number of components in vector `ResultComponent[m - 1]`. |

Above parameters are required for the workload in the specified order. A backend must specify, at least, a set of default arguments for these parameters.

Backends can require extra parameters beyond the base requirements. If a backend requires extra parameters, these must have default values in every set of default arguments for the workload parameters. Extra workload parameters are always listed after the set of required parameters.

## Categories <a name="categories"></a>
This workload supports the following categories:

```cpp
hebench:APIBridge::Category::Latency
hebench:APIBridge::Category::Offline
```

### Category Parameters <a name="category-parameters"></a>
#### Latency
See `hebench::APIBridge::CategoryParams::latency`.

#### Offline
See `hebench::APIBridge::CategoryParams::offline`.

Value ranges for elements in `CategoryParams::offline::data_count`. Default value is used for elements that take any sample size, but no sample size or `0` is specified in the hierarchy by Test Harness.

| Parameter | Lower bound | Upper bound | Default |
|-|-|-|-|
| `i` | `1` | none | `1` | 

## Data Type <a name="data-type"></a>

The data type defines the type for the scalar elements in each vector of the input and the output.

This workload is defined for the following data types:

```cpp
hebench::APIBridge::DataType::Int32
hebench::APIBridge::DataType::Int64
hebench::APIBridge::DataType::Float32
hebench::APIBridge::DataType::Float64
```

## Data Layout <a name="data-layout"></a>
All scalar elements in a vector with elements of type `T` (where `T` is any of the supported types) lie contiguous in memory.

For example, given the following vector `Z` with `n = 3` components:

```cpp
Z = [ a0, a1, a2 ]
```

The elements will be stored in memory as:

| Offset: | 0 | 1 | 2 |
|-|-|-|-|
|Z| `z0`  | `z1`  | `z2`  |

Backends should expect this layout for their raw, clear text inputs, and must generate this layout for their decoded outputs.

External datasets must provide their data for every input and output in a way that after going through the loading process as specified by the corresponding data loader, the data is laid out in memory in the aforementioned format as well.

#### Notes
If several vectors will be pointed at by a single pointer, consecutive vectors will follow each other in memory.

## Dataset <a name="dataset"></a>
Supported modes:

| Generate | External |
|-|-|
| no | yes |

Synthetic data cannot be generated for a generic workload since the `op` function is explicitly defined. If no dataset is provided in the benchmark configuration file, the benchmark initialization fails with an error.

## Example <a name="example"></a>
For our purpose, let's assume that the function we want to benchmark maps `(InputParam[0], InputParam[1])` to `(ResultComponent[0], ResultComponent[1], ResultComponent[2])` where `InputParam[0]` is a vector with two components, `InputParam[1]` is a vector with two components as well and:

```cpp
ResultComponent[0] = InputParam[0] + InputParam[1]
ResultComponent[1] = InputParam[0] - InputParam[1]
ResultComponent[2] = InputParam[0] . InputParam[1] // dot product
```

Such operation is not supported by the standard workloads defined by HEBench, so, we need a generic workload to measure the performance of our backend implementation.

Let's also assume that we want to test in *offline* category using a dataset with `2` samples for `InputParam[0]` and `3` samples for `InputParam[1]`.

### Example Configuration

These are the values that would be set in the [benchmark yaml configuration file](@ref config_file_reference).

#### Configuration for input InputParam[0]
- Number of samples: `2`


#### Configuration for input InputParam[1]
- Number of samples: `3`

#### Workload parameters

| Index | Name | Type | Value |
|-|-|-|-|
| `0` | `n` | `uint64_t` | `2` |
| `1` | `m` | `uint64_t` | `3` |
| `2` | `length_InputParam0` | `uint64_t` | `2` |
| `3` | `length_InputParam1` | `uint64_t` | `2` |
| `4` | `length_ResultComponent0` | `uint64_t` | `2` |
| `5` | `length_ResultComponent1` | `uint64_t` | `2` |
| `6` | `length_ResultComponent2` | `uint64_t` | `1` |

### Example Dataset

We must provide a dataset with our inputs and the expected output of the operation function evaluated on each input sample for validation purposes. The dataset must be provided in one of the dataset formats as provided by the [dataset loader](@ref dataset_loader_overview).

#### Samples for InputParam[0]

| Sample | Value |
|-|-|
| 0 |`(1, 2)`|
| 1 |`(-1, -1)`|

#### Samples for InputParam[1]

| Sample | Value |
|-|-|
| 0 |`(-1, 0)`|
| 1 |`(1, 1)`|
| 2 |`(2, -1)`|

#### Full dataset as must be provided to Test Harness

| Sample | Inputs | | Outputs | | |
|-|-|-|-|-|-|
|     | `InputParam[0]`   | `InputParam[1]`    | `ResultComponent[0]`   | `ResultComponent[1]`   | `ResultComponent[2]` |
| `0` | `(1, 2)` | `(-1, 0)` | `(0, 2)` | `(2, 2)` | `(-1)` |
| `1` | `(1, 2)` | `(1, 1)` | `(2, 3)` | `(0, 1)` | `(3)` |
| `2` | `(1, 2)` | `(2, -1)` | `(3, 1)` | `(-1, 3)` | `(0)` |
| `3` | `(-1, -1)` | `(-1, 0)` | `(-2, -1)` | `(0, -1)` | `(1)` |
| `4` | `(-1, -1)` | `(1, 1)` | `(0, 0)` | `(-2, -2)` | `(-2)` |
| `5` | `(-1, -1)` | `(2, -1)` | `(1, -2)` | `(-3, 0)` | `(-1)` |

