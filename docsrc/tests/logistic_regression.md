Logistic Regression Inference Workload {#logistic_regression}
========================

## Operation Description

The operation for this workload is the logistic regression with `sigmoid` activation function. 

### Summary
This operation is defined as:

```cpp
P(X) = op(W, b, X)
```

where

```cpp
op(W, b, X) = sigmoid(X . W + b)
```

### Operation Parameters

Input: `3` parameters

| Parameter | Description |
|-|-|
| `0` | `W` is the feature vector for the linear regression with `n` features (number of components in the vector). |
| `1` | `b` is the bias for the linear regression. This is a scalar. |
| `2` | `X` is the input vector to the linear regression. It has `n` components. |

### Result:

Output: `1` output

| Output | Description |
|-|-|
| `0` | `P(X)` is a scalar. It is the result of the logistic regression. |

### Details

Given the feature vector `W` and the bias `b`, for any input vector `X_`

```cpp
P(X = X_) = sigmoid(X_ . W + b)
```

where:
```cpp
x = X_ . W + b
```
is the result of the linear regression. `X_ . W` is the dot product between vectors `X_` and `W`, and

```cpp
double sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
}
```
is the standard sigmoid function.

## Workloads

This document applies to the following workloads:

```cpp
// From hebench/api_bridge/types.h

hebench:APIBridge::Workload::LogisticRegression
hebench:APIBridge::Workload::LogisticRegression_PolyD3
hebench:APIBridge::Workload::LogisticRegression_PolyD5
hebench:APIBridge::Workload::LogisticRegression_PolyD7
```

#### Notes
For `hebench:APIBridge::Workload::LogisticRegression_PolyD3`, `hebench:APIBridge::Workload::LogisticRegression_PolyD5`, and `hebench:APIBridge::Workload::LogisticRegression_PolyD7` the `sigmoid` activation is approximated around `x = 0` using a polynomial. The polynomial approximation used is<sup>[1]</sup>:

Degree 3:
```cpp
sigmoid_d3(x) = 0.5 + 1.20096(x/8) - 0.81562(x/8)^3
```

Degree 5:
```cpp
sigmoid_d5(x) = 0.5 + 1.53048(x/8) - 2.3533056(x/8)^3 + 1.3511295(x/8)^5
```

Degree 7:
```cpp
sigmoid_d7(x) = 0.5 + 1.73496(x/8) - 4.19407(x/8)^3 + 5.43402(x/8)^5 - 2.50739(x/8)^7
```

### Workload Parameters

Required workload parameters: `1`

| Index | Name | Type | Description |
|-|-|-|-|
| `0` | `n` | `uint64_t` | Number of features in feature vector `W`. This is the number of components for vectors `W` and `X`. |

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
| `0`* | `1` | `1` | `1` | 
| `1`* | `1` | `1` | `1` | 
| `2` | `1` | none | `100` |

&nbsp;* When set to `0`, these are set to the specified default values. Not affected by Test Harness sample size option.

## Data Type

This workload is defined for the following data types:

```cpp
hebench::APIBridge::DataType::Float32
hebench::APIBridge::DataType::Float64
```

## Data Layout

### 1. Vectors
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

### 2. Scalars
All scalars will be represented as vectors with a single element.

## Dataset
Supported modes:

| Generate | External |
|-|-|
| yes | yes |

Data generation for vectors used as input for this workload occurs during workload initialization by Test Harness. Ground truths are pre-computed during data generation. There is no standard dataset.

During data generation, all vector elements are extracted from a pseudo-random normal distribution of mean `0` and standard deviation of `1`: `n(0, 1)`.

## References
[1] -  M. Kim, et al. "Secure Logistic Regression Based on Homomorphic Encryption: Design and Evaluation" Cryptology ePrint Archive, Report 2018/074, 2018.
