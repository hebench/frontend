Adding a New Workload to Test Harness                {#extend_test_harness}
========================

[TOC]

Refer to @ref glossary for a list of terms that will be mentioned throughout this tutorial.

To add a new workload to HEBench, the first step is to define it, followed by adding it into the Test Harness. Since Test Harness is the program that tests backends on workloads, if it doesn't know the workload, it will not test for it.

### 1. Define a workload.

Defining a workload is an abstract process where the workload parameters, operations, categories, etc. are specified. The definition of a workload is the design that lives outside of HEBench.

See the definition of [Vector Element-wise Addition](@ref elementwise_add) workload for an example.

Next, once the workload is defined, HEBench must be made aware of it.

### 2. Declare the workload in the API Bridge.

To declare the workload in the API Bridge, a new enumeration value representing the workload <b>must be appended</b> to `enum hebench::APIBridge::Workload` in `hebench/api_bridge/types.h`.

At this point, Test Harness and any backends are able to use the workload as part of their workload description.

Note: make sure to change the API version values in file `VERSION` as appropriate to reflect changes in the API Bridge. Adding new workload, as explained, should only constitute a revision increase.

For appropriate API Bridge versioning, check <b>API Bridge Versioning General Guidelines</b> in @ref APIBridge_overview .

#### (Optional) Add a C++ wrapper for the workload parameters.
If the new workload requires flexible workload parameters, adding a wrapper to ease development of backends is optional, but encouraged.

To add a wrapper, extend class `hebench::cpp::WorkloadParams::Common` from `workload_params.h`. Check this header file for examples of other wrappers already implemented for existing workloads.

### 3. Implement workload in Test Harness.

This step entails the addition of the workload implementation into the Test Harness.

There are two ways to extend the Test Harness with a new workload. Which one is used depends on how much flexibility is required for the new workload and whether it fits in the predefined category behavior. For pre-defined behavior, see the first option. For full flexibility, check the second option.

#### Option 1: Pre-defined Category Behavior

If default behavior for any of the categories (**Latency**, **Offline**, etc.) is needed, then, the new workload benchmark class can inherit from the corresponding implementation of `IBenchmark`:

* ```hebench::TestHarness::BenchmarkLatency``` for [Latency](extend_test_harness_l.md)
* ```hebench::TestHarness::BenchmarkOffline``` for [Offline](extend_test_harness_o.md)

The `run()` method has already been implemented in each of these classes to follow the default measuring methodology for the category. See the corresponding documentation on extending from each and explanation of the default flow for each category as implemented by these classes.

#### Option 2: Full Flexibility (Advanced)

For full flexibility, inherit from `hebench::TestHarness::PartialBenchmark` to have common implementations to trivial methods declared by interface `IBenchmark`.
    
Method `hebench::TestHarness::PartialBenchmark::init()` must be implemented to perform initialization after construction, but before the backend itself is initialized. Data loading or generation should take place in this method. Errors during initialization should be reported as exceptions derived from `std::exception`.
    
Method `hebench::TestHarness::PartialBenchmark::postInit()` can be overriden to provide initialization that requires the backend to already exist. Note that overrides of this method must call the base method as their first operation.
    
The implementation of method `IBenchmark::run()` is the most important since it is the implementation of the actual benchmark. For correct statistical computations of reported events during the run, make sure that events of the same type are added to the `hebench::Utilities::TimingReportEx` `out_report` parameter under the same event ID, and the event which is the main benchmarking operation is clearly specified when adding the event type.

### 4. Make Test Harness aware of the new workload.

Once the workload is implemented, the final step is to make the Test Harness aware of it.

1. Describe the workload.

    To do so, first create a workload description class that derives from interface `hebench::TestHarness::PartialBenchmarkDescriptor` (from header `hebench_ibenchmark.h`) and implement all of its <b>abstract</b> methods following their interface documentation:
    
    * `hebench::TestHarness::IBenchmarkDescriptor::createBenchmark()`
    * `hebench::TestHarness::IBenchmarkDescriptor::destroyBenchmark()`
    * `hebench::TestHarness::PartialBenchmarkDescriptor::matchBenchmarkDescriptor()`
    * `hebench::TestHarness::PartialBenchmarkDescriptor::completeWorkloadDescription()`
    
    Inside `createBenchmark()`, the benchmark object should be constructed and returned. The object's initialization methods will be called after construction automatically, and thus, they should not be called manually by our implementation.
    
    Note that benchmark description objects should be lightweight because they will exist throughout the lifetime of the main Test Harness process.

2. Register workload description with the benchmark factory.

    Next, an object of the workload description class mentioned above has to be registered with the `hebench::TestHarness::BenchmarkFactory` before the Test Harness starts. This ensures that the factory knows of this new workload when it is creating benchmarks. This is accomplished by calling method `hebench::TestHarness::BenchmarkFactory::registerSupportedBenchmark()` <em>during static initialization</em>.
    
    The recommended way to do this is to define a static `bool` member in the workload description class created in the previous step, and, during declaration, initialize it with the return value of `BenchmarkFactory::registerSupportedBenchmark()` method, where the parameter for the method is a `shared_ptr` instantiation of said workload description.

Once registered, `BenchmarkFactory` will call the methods in benchmark description objects to pick the right benchmark for a workload, create it, and destroy it once the benchmark completes execution.

For an example of registering a benchmark workload with the `BenchmarkFactory` and all the corresponding nuances, check the mechanism implementation in `hebench_eltwiseadd_l.h` and `hebench_eltwiseadd_l.cpp`.
