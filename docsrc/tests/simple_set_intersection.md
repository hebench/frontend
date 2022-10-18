Simple Set Intersection Workload {#simple_set_intersection}
========================

[TOC]

## Operation Description

### Summary
This operation is defined as:

```cpp
Z = op(X, Y)
```

where

```
op(X, Y) = X ⋂ Y
```

and operation `⋂` is the simple set intersection.

### Operation Parameters

Input: `2` parameters

| Parameter | Description |
|-|-|
| `0` | `X` is a dataset containing `n` items. |
| `1` | `Y` is a dataset containing `m` items. |

### Result:

Output: `1` output

| Output | Description |
|-|-|
| `0` | `Z` is a set with, at most `min(n, m)` items, where every item in `Z` is present in both `X` and `Y`. |

### Details

If `A` is a set, and `a_i` is an element in `A`, then, the standard simple set intersection operation is defined as:

```
Z = { z_i; where z_i ∈ X and z_i ∈ Y }
```

## Workloads

This document applies to the following workloads:

```cpp
// From hebench/api_bridge/types.h

hebench:APIBridge::Workload::SimpleSetIntersection
```

### Workload Parameters

Required workload parameters: `2`

| Index | Name | Type | Description |
|-|-|-|-|
| `0` | `n` | `uint64_t` | Size of dataset `X`. |
| `1` | `m` | `uint64_t` | Size of dataset `Y`. |
| `2` | `k` | `uint64_t` | Number of elements in an item of a dataset. |

A backend must specify, at least, a set of default arguments for these parameters.

Backends can require extra parameters beyond the base requirements. If a backend requires extra parameters, these must have default values in every set of default arguments for the workload parameters.

## Categories
This workload supports the following categories:

```cpp
hebench:APIBridge::Category::Latency
hebench:APIBridge::Category::Offline
```

### Category Parameters
#### Latency
See `hebench::APIBridge::CategoryParams::latency`.

#### Offline
See `hebench::APIBridge::CategoryParams::offline`.

Value ranges for elements in `CategoryParams::offline::data_count`. Default value is used for elements that take any sample size, but sample size of `0` is specified by Test Harness.

| Parameter | Lower bound | Upper bound | Default |
|-|-|-|-|
| `0` | `1` | none |`100` | 
| `1` | `1` | none |`100` | 

## Data Type

This workload is defined for the following data types:

```cpp
hebench::APIBridge::DataType::Int32
hebench::APIBridge::DataType::Int64
hebench::APIBridge::DataType::Float32
hebench::APIBridge::DataType::Float64
```

## Data Layout

### 1. Sets

If data type is `hebench::APIBridge::DataType::String`, each character of the string will be encoded as a 32-bits integer.
 
All the items in a set are vectors with elements of type `T` (where `T` is any of the supported numeric types). Every element of an item lies contiguous in memory, and every item also lies contiguous in memory.

For example, given the following set `X` with `n = 3`, `k = 2` and a set being represented by a vector components:

```cpp
X = { [ x_00, x_01], [x_10, x_11], [x_20, x21] }
```

The elements will be stored in memory as:

| Offset: | 0 | 1 | 2 | 3 | 4 | 5|
|-|-|-|-|-|-|-|
|X| `x_00` | `x_01` | `x_10` | `x_11` | `x_20` | `x21`  |

For this case, backends should expect this layout for their raw, clear text inputs, and must generate this layout for their decoded outputs.

## Dataset

Supported modes:

| Generate | External |
|-|-|
| yes | yes |

### Data Generation
Data generation for sets used as input for this workload occurs during workload initialization by Test Harness. Ground truths are pre-computed during data generation. There is no standard dataset.

During data generation, all set elements are extracted from a pseudo-random uniform distribution between `-16384` and `16384`: `u(-16384, 16384)`.

### External Datasets
From external datasets, if an item is shorter than `k`, it will be padded. If an item is longer than `k` it will be truncated and a warning will be displayed.
