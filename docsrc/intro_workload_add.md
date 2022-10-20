Working with Unsupported Workloads {#intro_workload_addition}
========================

[TOC]

At some point a user may need to benchmark a workload that is not currently supported by Test Harness. While creating a backend for and benchmarking currently supported workloads is easy, and it is the intended way of using HEBench, benchmarking an unsupported workload is still possible.

There are two ways to benchmark an unsupported workload:

- [Create a custom workload using the Generic Workload operation](@ref generic_workload)
  
  This does not require changing the code base for the framework. Users need to configure the custom workload using a configuration file, and supply a custom dataset that defines the workload inputs and expected outputs, (the dataset must be specified as the data should be laid out in memory; i.e. ordering, padding, etc.). Once configured, users can provide backends that operate on the custom workload as if it were any other supported workload. Test Harness uses the configuration and dataset to communicate information to the backend.
- [Add support for the new workload into Test Harness](@ref extend_test_harness)
  
  This method requires modifying the framework to extend Test Harness in order to provide the implementation of the new workload. This is useful to add workloads that may be difficult to match to the expected structure of the Generic Workload. However, this method requires knowledge of the framework code base and it is an advanced topic.
