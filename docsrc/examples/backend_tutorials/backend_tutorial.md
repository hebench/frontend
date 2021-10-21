Quickstart C++ Wrapper Backend Tutorial {#simple_cpp_example}
========================
This tutorial details the steps involved in implementing a new backend engine utilizing the [C++ wrapper](@ref CPP_overview) for use with the testing framework.

This guide is intended to be a beginers guide for people who are looking to get a backend with a single test implemented quickly. It is based on using the `api_bridge_example_backend` as a starting point for a new backend implementation which uses the C++ wrapper.

For the purposes of this guide, we will be creating a very basic backend implementing a benchmark for the **vector element-wise addition** workload, under the **offline** category, with **first operand in plain text and second encrypted**.

<hr/>
Select your preferred library below. The tutorial will follow up using that library to perform Homomorphic Encryption (HE) operations.

[C++ Wrapper Backend Tutorial using PALISADE lattice encryption library]( @ref simple_cpp_example_palisade) <br/>
[C++ Wrapper Backend Tutorial using Microsoft SEAL library](@ref simple_cpp_example_seal)
