
Simple Set Intersection Workload {#simple_set_intersction}
========================

[TOC]

## Operation Description

### Summary
This operation is defined as:

```cpp
Z = op(X, Y)
```

where

```cpp
$$op(X, Y) = X \bigcap Y$$
```

and operation `\bigcap` is the simple set intersection.

### Operation Parameters

Input: `2` parameters

| Parameter | Description |
|-|-|
| `0` | `X` is a set containing  elementes. |
| `1` | `Y` is a set containing  elementes. |

### Result:

Output: `1` output

| Output | Description |
|-|-|
| `0` | `Z` is a set. It is the result of the intersection between the sets `X` and `Y`. |

### Details

if `x_i` is present in `X` and in `Y`, then it will be part of `Z`, then, the standard simple set intersection operation is defined as:

```
Z = {`$x_1$`, ..., `$x_n$` | `$x_i$` $\in$ `$X$` $\wedge$ `$x_i$` $\in$ `$Y$`  }
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
| `0` | `n` | `uint64_t` | Number of elements in the first set. |
| `1` | `m` | `uint64_t` | Number of elements in the second set. |

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
hebench::APIBridge::DataType::String
```

## Data Layout

### 1. Sets
All the members in a set with elements of type `T` (where `T` is any of the supported types) lie contiguous in memory.

For example, given the following set `X` with `n = 3` and a set being represented by a vector components:

```cpp
X = { x_0, x_1, x_2 }
```

The elements will be stored in memory as:

| Offset: | 0 | 1 | 2 |
|-|-|-|-|
|X| `x_0`  | `x_1`  | `x_2`  |

For this case, backends should expect this layout for their raw, clear text inputs, and must generate this layout for their decoded outputs.

## Dataset

Supported modes:

| Generate | External |
|-|-|
| yes | yes |

Data generation for sets used as input for this workload occurs during workload initialization by Test Harness. Ground truths are pre-computed during data generation. There is no standard dataset.

During data generation, all set elements are extracted from a pseudo-random normal distribution of mean `0` and standard deviation of `10`: `n(0, 10)`.
