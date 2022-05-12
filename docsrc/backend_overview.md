HEBench Backend Overview                {#backend_overview}
========================

[TOC]

## Overview

The HEBench components that enable backends consist of the API Bridge and the C++ wrapper. These enable the creation of backends to implement the benchmarks for [supported workloads](@ref tests_overview).
 
Backend engines are expected to be shared libraries (.so in Linux, .dll in Windows) that expose the [API Bridge](@ref APIBridge_overview) functionality. The functionality can be implemented by directly defining the required functions specified in the @ref hebench::APIBridge or extending the [C++ wrapper](@ref CPP_overview).

Backends are free to implement any combination of test and datatype combinations and register these tests as part of their implementation.

Creating backends for different hardware and software libraries is the expected primary way in which submitters will interface with HEBench.

<div align="center">
  <img width="450" src="architecture_diagram_backend_1.png" /><br>
  <span>Figure 1. : HEBench backend component diagram</span>
</div>

The tutorial at the end of this page is the fastest way for users to get started in the process of creating a backend to benchmark their implementation of one or more of the supported workloads.

## Components

 - @ref APIBridge_overview : Describes the details of the backends C API.
 - @ref CPP_overview : Describes the details of the backends C++ API wrapper.
 - @ref tests_overview : list of all currently implemented tests with detailed description for each one.
 
## Tutorials

 - @ref simple_cpp_example : A quick start example showing how to implement a simple backend by extending the C++ wrapper.
