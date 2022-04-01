HEBench Backend Overview                {#backend_overview}
========================

[TOC]

##Overview
The HEBench components that enable backends consist of the API Bridge and the C++ wrapper that enable the creation of backends that implement the benchmarks for the supported workloads.
 
Backend engines are expected to be shared libraries (.so in Linux, .dll in Windows) that expose the [API Bridge](@ref APIBridge_overview) functionality. The functionality can be implemented by directly defining the required functions specified in the @ref hebench::APIBridge or extending the [C++ wrapper](@ref CPP_overview).

Backends are free to implement any combination of test and datatype combinations and register these tests as part of their implementation.

Creating backends for different hardware and software libraries is the expected primary way in which submitters will interface with HEBench.

<div align="center">
  <img width="450" src="architecture_diagram_backend_1.png" /><br>
  <span>Figure 1. : HEBench backend component diagram</span>
</div>

##Components
 - @ref APIBridge_overview : Describes the details of the backends C API.
 - @ref CPP_overview : Describes the details of the backends C++ API wrapper.
 
##Tutorials
 - @ref simple_cpp_example : A quick start example showing how to implement a simple backend by extending the C++ wrapper.
