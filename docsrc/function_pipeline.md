API Bridge Function Pipeline Chart {#function_pipeline_chart}
========================

API Bridge functions are called by the frontend's Test Harness as part of the testing procedure. Each function will recieve some parameters as input, perform some expected operation in the backend, and then pass the results back to the Test Harness which will use the returned results as input to later functions in the flow.

This chart depicts the pipeline.

<div align="center">
  <img width="750" src="function_pipeline.png" /><br>
  <span>Figure 1. : API Bridge Function pipeline flow chart.</span>
</div>

Arrows indicate the data flow: the tail of an arrow indicates that the output of that function may be used as input to the function at the head of the arrow, and thus, they should be compatible.

Note that green arrows specify the default flow for a standard benchmark and must be supported by all backends.

Yellow arrows show optional dependencies, and unless specified by a workload or categories they do not need to be implemented by backends.

More detailed information can be found in each specific API Bridge function documentation.
