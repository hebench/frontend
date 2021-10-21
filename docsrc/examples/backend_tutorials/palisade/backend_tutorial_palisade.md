Quickstart C++ Wrapper Backend Tutorial - PALISADE {#simple_cpp_example_palisade}
========================
## Backend Creation Tutorial
This tutorial details the steps involved in implementing a new backend utilizing the [C++ wrapper](@ref CPP_overview) for use with the testing framework.

This guide is intended to be a beginners guide for people who are looking to get a backend with a single test implemented quickly. It is based on using the `api_bridge_example_backend` as a starting point for a new backend implementation which uses the C++ wrapper.

For the purposes of this guide we will be creating a very basic backend implementing a benchmark for the **vector element-wise addition** workload, under the **offline** category, with **first operand in plain text and second encrypted**, using PALISADE v1.11.3 to perform Homomorphic Encryption (HE) operations.

Assume that we already have a functional workflow for our workload as listed below, and we want to benchmark it using HEBench.
