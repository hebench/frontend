\mainpage HEBench Documentation Main Page

##Overview

HEBench is designed to be a flexible, extendable, intuitive, and reliable framework for testing and evaluating homomorphic encryption software and hardware solutions. It defines an abstraction API in C to enable integration with the widest variety of hardware and software and executes high level tests using a front end application which feeds data to supported HE backends through the APIBridge component, generating detailed performance reports and verifying the accuracy of the returned results. 

##Features
 - Support for a wide range of software and hardware through highly flexible C API.
 - Automatic test data generation and results verification
 - Comprehensive performance metric report generation
 - Dynamic backend loading for easy comparison. 
 - Includes front end support for a wide range of tests of interest to the Homomorphic Encryption space such as
     - Matrix Multiplication
     - Element wise vector addition.

##Project Architecture
<div align="center">
  <img width="900" src="architecture_diagram.png" /><br>
  <span>Figure 1. : HEBench architecture diagram</span>
</div>
Figure 1 above shows a high-level diagram of the project architecture and its components. We further separate these components into what we call the frontend and backend components based upon how we expect users will interact with the project. Details on the backend and frontend are described in detail on their respective overview pages.

##HEBench Backend
A HEBench backend is where the API and helper classes for interfacing with different homomorphic encryption (HE) hardware and software are defined.

<b>Submitters</b> are the primary users expected to interface with the HEBench project. To benchmark their implementation of one or more workloads supported by the frontend, submitters must create a backend for HEBench by implementing the C API Bridge (or extending the C++ wrapper).

Users interested in benchmarking their solution to supported HE workloads should start here. The tutorial provided in the backend overview section should offer a quick starting point.

- @ref backend_overview : provides details on backends including components, and detailed information on how to create new backends to test implementations for the HE workloads.

##HEBench Frontend

The HEBench frontend is where HE tests are defined, scheduled for execution, and results collected by the Test Harness.

Test Harness is the main component of the frontend. It is the main executable that benchmarks the performance of supported HE workloads implemented in backends.

Advanced users that wish to extend the frontend by implementing new tests and adding new workloads are <b>Collaborators</b>. Frontend provides an extensible framework that allows for these additions.

To become familiar with the functionality and features of Test Harness, users should start here. Advanced users interested in collaborating and adding new benchmarks to the frontend should start here as well.

- @ref frontend_overview : provides details on the frontend including components, how to use, supported tests, and how to extend it.
	
##HEBench Component namespace quick references 

This section provides links to the core components and sections of the HEBench project. 
 - The API Bridge specifices the C API which testing backends are required to implement. It is defined in the hebench::APIBridge namespace. 
     - [API Bridge namespace reference](@ref hebench::APIBridge)
 - The HEBench C++ wrapper is a helper library which wraps the C API Bridge in a set of helper C++ classes to make it easier to implement backends. The classes and components of this module are defined under the hebench::cpp namespace. 
     - [HEBench C++ wrapper namespace reference](@ref hebench::cpp)
 - The test harness implements and executes tests which are executed on supporting backends which implement the APIBridge API. The test harness and implemented tests are defined under the hebench::TestHarness namespace. 
     - [Test Harness namespace reference](@ref hebench::TestHarness)

