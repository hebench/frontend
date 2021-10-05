HEBench frontend overview                {#frontend_overview}
========================

##Description
The HEBench frontend consists of the components relevant to users running benchmarks, extending the test harness with their own benchmarks, and developers who wish to contribute to the testing side of the framework.

<div align="center">
  <img width="450" src="architecture_diagram_frontend_1.png" /><br>
  <span>Figure 1. : HEBench frontend component diagram</span>
</div>

##Components
The front end is comprised of the following components
 - @ref test_harness_overview : implements and executes tests which are executed on supporting backends which implement the APIBridge API. Builds the executable benchmarking application
 - @ref tests_overview : List of all currently implemented tests with detailed description for each one.

##Tutorials