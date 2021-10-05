Quickstart C++ Wrapper Backend Tutorial {#simple_cpp_example}
========================
##Backend Engine addition tutorial
This tutorial details the steps involved in implementing a new backend engine utilizing the C++ wrapper for use with the testing framework. 
This guide is intended to be a beginers guide for people who are looking to get a backend with a single test implemented quickly. It is based on using the `api_bridge_example_backend` as a starting point for a new backend implementation which uses the [C++ wrapper](@ref CPP_overview).

For the purposes of this guide we will be creating a very basic backend implementing a benchmark for the **vector element-wise addition** workload, under the **offline** category, with **first operand in plain text and second encrypted**, using Microsoft SEAL to perform Homomorphic Encryption (HE) operations.

Follow the tutorial steps in order to complete it.

## Tutorial steps

<b>Tutorial Home</b><br/>
[Preparation](backend_tutorial_preparation.md)<br/>
[Engine Initialization and Benchmark Description](backend_tutorial_init.md)<br/>
[Benchmark Implementation](backend_tutorial_impl.md)<br/>
[File References](backend_tutorial_files.md)
