Latency Category                {#category_latency}
========================

[TOC]

Latency is one of the HEBench supported benchmarking categories.

See the complete [list of benchmarking categories](@ref category_overview) for other options.

## Category Flag

```cpp
hebench:APIBridge::Category::Latency
```

## Category Parameters

- `std::uint64_t min_test_time_ms`: supported by all categories.
  Specifies the minimum time, in milliseconds, to run the test.

- `std::uint64_t warmup_iterations_count`:
  Specifies the number of warmup iterations to perform before running the actual test.

Note that a backend implementation that sets any field in `CategoryParams` to `0` or `null` values (as corresponding to the field data type) is indicating that it accepts any valid value in the supported range for said field (according to the valid ranges as specified in the documentation for each field). In this case, these values are user-configurable using Test Harness features such as [benchmark configuration files](@ref config_file_reference).

## Description

The Latency benchmarking category measures the end-to-end time to complete a workload operation with a single input.

Latency is a deterministic computation category: workloads that support this category must be able to produce the exact same results given the same input values.

During latency test, Test Harness passes a single input sample to the backend. When testing, more than one call will be issued to `hebench::APIBridge::operate()`. Calls will occur until the test time specified during benchmark configuration has elapsed (at least two calls will always occur). The same input sample is used during every call; therefore, the backend should create a copy of the input before operating when the implementation modifies or destructs the values in the input. The objective is for Test Harness to measure how fast can the implementation perform the operation on a single input sample. The repetition is intended to collect statistically significant results.

To correctly measure latency, the implementation of the backend `hebench::APIBridge::operate()` function should not cache results of previous calls, intermediate, final, or any other kind. This is, results must be computed every time, regardless of whether the input sample are the same as inputs from previous calls. The backend implementation must make sure that the operation actually starts running when the `operate()` function is called, not when the `hebench::APIBridge::load()` function completes. Function `hebench::APIBridge::store()` is provided to retrieve the results of the operation; however, the backend implementation must also ensure that `operate()` does not return until the operation completes and results are ready to be retrieved, not before. For example, if the operation will execute asynchronously, it should start only when `operate()` function is called, and the implementation of `operate()` should block until the asynchronous operation completes.

If the implementation requires warm up iterations, it can request so via benchmark configuration file, or enforce it with the `warmup_iterations_count` field from `hebench::APIBridge::CategoryParams` when filling out the `hebench::APIBridge::BenchmarkDescriptor` structure in the backend. Warmup iterations are calls to `hebench::APIBridge::operate()` exactly as if they were being tested, but they do not affect the measured results. Note that Test Harness still measures the time for the warmup; it is just reported in its own entry in the benchmark report.
