HEBench Test Harness Overview                {#test_harness_overview}
========================
##Description
The Test Harness is the is the core frontend component and user interaction point for the HEBench project. It is built as part of the HEBench frontend and it is responsible for orchestrating the benchmarking operations, submiting and timing requests to backends, parsing user arguments and configurations, and outputting benchmark reports and summaries. It provides support for dynamically loading compatible HEBench backends and benchmarking supported tests. 

##User Guides

The documents and pages listed in this section detail how to use the test harness, including how to run workloads for different backends, configure workloads, available commandline options, etc.
 
 - @ref test_harness_usage_guide
 - @ref config_file_reference

##Namespace
[Test Harness namespace](@ref hebench::TestHarness)

##Tutorials

The documents and guides in this section detail how to extend the test harness. Including how to add new workloads, test types, etc.

**NOTE**: Extending the Test Harness is an advanced topic. The primary usage for HEBench is expected to be the creation of backends and benchmarking them with the existing Test Harness.

This section is for advanced users that require other tests, workloads, or categories not currently supported by the Test Harness and API Bridge.

 - @ref extend_test_harness
