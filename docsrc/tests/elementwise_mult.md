Vector Element-wise Multiplication Workload {#elementwise_mult}
========================

## Operation Description

### Summary
This operation is defined as:

```cpp
C = op(A, B)
```

where `op` is the standard vector element-wise multiplication.

### Operation Parameters

Input: `2` parameters

| Parameter | Description |
|-|-|
| `0` | `A` is a vector with `n` components. |
| `1` | `B` is a vector with `n` components. |

### Result:

Output: `1` output

| Output | Description |
|-|-|
| `0` | `C` is a vector with `n` components. It is the result of the component-wise multiplication of `A` and `B`. |

### Details

If `Z[i]` denotes the element at component `i` in vector `Z`, then, the standard element-wise multiplication operation is defined as:

```
C[i] = A[i] * B[i]
```

## Workloads

This document applies to the following workloads:

```cpp
// From hebench/api_bridge/types.h

hebench:APIBridge::Workload::EltwiseMultiply
```

### Workload Parameters

| Index | Name | Type | Description |
|-|-|-|-|
| `0` | `n` | `uint64_t` | Number of components in a vector. |

Above parameters are required for the workload in the specified order. A backend must specify, at least, a set of default arguments for these parameters.

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
All scalar elements in a vector with elements of type `T` (where `T` is any of the supported types) lie contiguous in memory.

For example, given the following vector `A` with `n = 3` components:

```cpp
A = [ a0, a1, a2 ]
```

The elements will be stored in memory as:

| Offset: | 0 | 1 | 2 |
|-|-|-|-|
|A| `a0`  | `a1`  | `a2`  |

Backends should expect this layout for their raw, clear text inputs, and must generate this layout for their decoded outputs.

#### Notes
If several vectors will be pointed at by a single pointer, consecutive vectors will follow each other in memory.

## Dataset
Supported modes:

| Generate | External |
|-|-|
| yes | yes |

Data generation for vectors used as input for this workload occurs during workload initialization by Test Harness. Ground truths are pre-computed during data generation. There is no standard dataset.

During data generation, all vector elements are extracted from a pseudo-random uniform distribution between `-10` and `10`: `u(-10, 10)`.
