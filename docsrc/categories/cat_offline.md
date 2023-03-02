Offline Category                {#category_offline}
========================

[TOC]

Offline is one of the HEBench supported benchmarking categories.

See the complete [list of benchmarking categories](@ref category_overview) for other options.

## Category Flag

```cpp
hebench:APIBridge::Category::Offline
```

## Category Parameters

- `std::uint64_t min_test_time_ms`: supported by all categories.
  Specifies the minimum time, in milliseconds, to run the test.

- `std::uint64_t data_count[HEBENCH_MAX_OP_PARAMS]`
  Specifies a hard value for the number of data samples for each input parameter to the workload operation.

Note that a backend implementation that sets any field in `CategoryParams` to `0` or `null` values (as corresponding to the field data type) is indicating that it accepts any valid value in the supported range for said field (according to the valid ranges as specified in the documentation for each field). In this case, these values are user-configurable using Test Harness features such as [benchmark configuration files](@ref config_file_reference).

## Description

The Offline benchmarking category measures throughput (samples per second) that a workload implementation can perform on a single batch of data. This is how fast the backend can process all the input, given all the data at once.

During offline test, Test Harness passes all the input samples (the complete dataset) to the backend for the backend implementation to encode as necessary to take full advantage of optimization techniques. When testing, the complete dataset is requested to be loaded through a call to `hebench::APIBridge::load()` in an attempt to give a chance to the backend implementation to isolate loading times from operation times. Once loaded, Test Harness will issue calls to `hebench::APIBridge::operate()`. Calls will occur until the test time specified during benchmark configuration has elapsed (at least one call will always occur). For every call, the same input handle for the dataset as returned by `load()` is used; therefore, the backend should create a copy of the inputs before operating when the implementation modifies or destructs the values in the input. The objective is for Test Harness to measure how fast can the implementation operate on bulk data. The repetition is intended to collect statistically significant results.

To correctly measure throughput, the rules are similar as those described for a latency test. The implementation of the backend `hebench::APIBridge::operate()` function should not cache results of previous calls, intermediate, final, or any other kind. This is, results must be computed every time, regardless of whether the input samples are the same as inputs from previous calls. The backend implementation must make sure that the operation actually starts running when the `operate()` function is called, not when the `hebench::APIBridge::load()` function completes. Function `hebench::APIBridge::store()` is provided to retrieve the results of the operation; however, the backend implementation must also ensure that `operate()` does not return until the operation completes and results are ready to be retrieved, not before. For example, if the operation will execute asynchronously, it should start only when `operate()` function is called, and the implementation of `operate()` should block until the asynchronous operation completes.

A backend implementation must keep results alive from every call to `operate()` as long as the corresponding result handle has not been destroyed (through a call to `hebench::APIBridge::destroyHandle()` ).

Warmup iterations are not available for offline tests.

## Ordering of Results

Many workloads in HEBench require more than one input parameter. Element-wise Vector Addition, for example, requires two input parameters (the two vectors to add).

When testing the throughput of a workload that requires more than one input parameter in the offline category, Test Harness may pass a collection of samples for each component. A backend that implements offline testing should be aware of how the component input samples are organized to form an input sample so that it can organize the results correctly for validation.

The following is the result ordering reference with respect to input batches as defined for HEBench:

@ref results_order

