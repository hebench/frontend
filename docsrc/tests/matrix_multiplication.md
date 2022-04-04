Matrix Multiplication Workload {#matrix_multiplication}
========================

[TOC]

## Operation Description

### Summary
This operation is defined as:

```cpp
M = op(M0, M1)
```

where

```cpp
op(M0, M1) = M0 x M1
```

is the standard matrix multiplication operation.

### Operation Parameters

Input: `2` parameters

| Parameter | Description |
|-|-|
| `0` | `M0` is a matrix with `m` rows and `n` columns (`m` x `n`). |
| `1` | `M1` is a `n` x `p` matrix. |

### Result:

Output: `1` output

| Output | Description |
|-|-|
| `0` | `M` is a `m` x `p` matrix. It is the result of multiplying `M0` and `M1`. |

### Details

If `A[i][j]` denotes the element at row `i` and column `j` in matrix `A`, then, the standard matrix multiplication operation is defined as:

```
M[i][j] = M0[i][1] * M1[1][j] + M0[i][2] * M1[2][j] + ... + M0[i][n] * M1[n][j]
```

## Workloads

This document applies to the following workloads:

```cpp
// From hebench/api_bridge/types.h

hebench:APIBridge::Workload::MatrixMultiply
```

### Workload Parameters

Required workload parameters: `3`

| Index | Name | Type | Description |
|-|-|-|-|
| `0` | `m` | `uint64_t` | Number of rows in matrix `M0`. |
| `1` | `n` | `uint64_t` | Number of columns in matrix `M0`. |
| `2` | `p` | `uint64_t` | Number of columns in matrix `M1`. |

Number of rows in `M1` is the same as number of columns in `M0` as per definition of the operation.

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
Data layout in memory for a matrix with elements of type `T` (where `T` is any of the supported types),  follows a row major ordering. All scalar elements in a matrix lie contiguous in memory, where consecutives elements in a row reside next to each other.

For example, given the following matrix `A (2 x 3)`:

```cpp
     /  a00  a01  a02 \
A = |                  |
     \  a10  a11  a12 /
```

The elements will be stored in memory as:

| Offset: | 0 | 1 | 2 | 3 | 4 | 5 |
|-|-|-|-|-|-|-|
|A| `a00`  | `a01`  | `a02`  | `a10` | `a11` | `a12`  |

Backends should expect this layout for their raw, clear text inputs, and must generate this layout for their decoded outputs.

#### Notes
If several matrices will be pointed at by a single pointer, consecutive matrix will follow each other in memory.

## Dataset
Supported modes:

| Generate | External |
|-|-|
| yes | yes |

Data generation for vectors used as input for this workload occurs during workload initialization by Test Harness. Ground truths are pre-computed during data generation. There is no standard dataset.

During data generation, all matrix elements are extracted from a pseudo-random normal distribution with mean `0` and standard deviation of `10`: `n(0, 10)`.
