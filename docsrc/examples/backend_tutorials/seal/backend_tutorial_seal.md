Quickstart C++ Wrapper Backend Tutorial - SEAL {#simple_cpp_example_seal}
========================
## Backend Creation Tutorial
This tutorial details the steps involved in implementing a new backend utilizing the [C++ wrapper](@ref CPP_overview) for use with the testing framework.

This guide is intended to be a beginners guide for people who are looking to get a backend with a single test implemented quickly. It is based on using the `api_bridge_example_backend` as a starting point for a new backend implementation which uses the C++ wrapper.

For the purposes of this guide we will be creating a very basic backend implementing a benchmark for the **vector element-wise addition** workload, under the **offline** category, with **first operand in plain text and second encrypted**, using Microsoft SEAL 3.6 to perform Homomorphic Encryption (HE) operations.

Assume that we already have a functional workflow for our workload as listed below, and we want to benchmark it using HEBench.

\snippet{lineno} docsrc/examples/backend_tutorials/seal/tutorial_backend/original_flow_seal.cpp tutorial original_flow seal

What we have to do is extract the stages of the testing pipeline from our workflow and integrate them into the C++ wrapper.

Follow the tutorial steps in order to complete it.

## Tutorial steps

<b>Tutorial Home</b><br/>
[Preparation](backend_tutorial_preparation_seal.md)<br/>
[Engine Initialization and Benchmark Description](backend_tutorial_init_seal.md)<br/>
[Benchmark Implementation](backend_tutorial_impl_seal.md)<br/>
[File References](backend_tutorial_files_seal.md)
