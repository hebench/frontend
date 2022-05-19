Quickstart C++ Wrapper Backend Tutorial - Engine Initialization and Benchmark Description {#be_tutorial_init_palisade}
========================

[TOC]

## Steps
    
### 1. Benchmark description initialization

The first step for our new benchmark is to fill out the benchmark description. `TutorialEltwiseAddBenchmarkDescription` defined in `tutorial_eltwiseadd_benchmark_palisade.h` inherits from `hebench::cpp::BenchmarkDescription`.

In the constructor we should initialize and set the inherited `m_descriptor` object. We currently have a description already in place from the original example to which we will make some changes for our PALISADE-based workload.

`TutorialEltwiseAddBenchmarkDescription` is declared in `tutorial_eltwiseadd_benchmark_palisade.cpp`.

First, we initialize the descriptor object memory:

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor init
    
**hebench::APIBridge::BenchmarkDescriptor.workload**: specifies which workload this benchmark will perform. Workloads supported by your current HEBench version are defined under `hebench::APIBridge::Workload` in `hebench/api_bridge/types_palisade.h`. For our example we will set it to `hebench::APIBridge::Workload::EltwiseAdd`:

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor workload
    
**hebench::APIBridge::BenchmarkDescriptor.data_type**: tells the front end what type of data the benchmark is expecting to recieve and return. The possible types are defined in `hebench::APIBridge::DataType`.

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor data_type
    
**hebench::APIBridge::BenchmarkDescriptor.category**: Specifies one of the benchmark categories defined under `hebench::APIBridge::Category`. For our first benchmark we are setting it to **offline**.
    
\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor category
    
**hebench::APIBridge::BenchmarkDescriptor.cat_params.offline**: These are parameters which are relevant to tests of the offline category. These specify the supported number of samples (or batch sizes) for each of the operation parameters. These are arbitrary and specific to the benchmark we want to create. See the documentation for more details.

Since our initial workflow forced sample sizes of `2` samples for the first operand and `5` for the second, we are setting them the same here. If our benchmark can support variable sample sizes for any operand, we set the corresponding value to `0`. The specific size can be set in a configuration file passed when running this benchmark. For more information regarding benchmark configuration files see @ref config_file_reference .
    
\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor category_params
    
**hebench::APIBridge::BenchmarkDescriptor.cipher_param_mask**: Specifies which operands will be plain text and which will be encrypted. This is a bit mask where the bit position represents the operation parameter, and its value represents whether it is plain text (`0`) or encrypted (`1`). For this tutorial we want first operand plain text and second encrypted.
    
\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor op_params_mask

Note that we are setting the mask to `1 << 1` or `0000 0010` in binary, indicating that operation parameter in position `0` is plain text, and postion `1` is encrypted. The rest of the bits are ignored if there are not corresponding operation parameters.
    
**hebench::APIBridge::BenchmarkDescriptor.scheme**: Specifies the scheme the test will use. It must be one of the schemes registered during engine initialization earlier.

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor scheme
    
**hebench::APIBridge::BenchmarkDescriptor.security**: Specifies what security level the benchmark uses. It must be one of the security levels registered during engine initialization earlier.
    
\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor security
    
**hebench::APIBridge::BenchmarkDescriptor.other**: This is an optional parameter which can be used for a backend defined purpose to differentiate benchmarks in additional backend-specific ways.

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor other

Finally, a workload that requires flexible workload parameters must specify, at least, one set of default arguments. Workload parameters are flexible enough for backends to specify other parameters beyond the mandatory requirements, such as, but not limited to parameters that could be used for optimization like padding size, extra memory, etc.

According to @ref elementwise_add , element-wise addition requires the number of elements for a vector. For this tutorial, the required parameter is enough, so, we do not need to create any extra workload parameters.

We create a single default set with a default value for the vector size:

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor workload_params
        
Since the value in our original workload was `400`, we are setting it the same as default. If our implementation happens to support only vectors of this size, we can enforce it during benchmark initialization itself.

The complete implementation of our Benchmark description is provided below
\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc constructor

### 2. Benchmark description overrides

We must override methods to create and destroy our actual benchmark. These will be called by C++ wrapper's BaseEngine implementation when the benchmark needs to be instantiated to execute, and disposed of when completed.

Our create method follows:

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc create

This is the destroy method:

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc destroy

There are other methods that we can override to modify the behavior of the description class. Check the documentation on `hebench::cpp::BenchmarkDescription` for more information.

In this tutorial we are overriding `hebench::cpp::BenchmarkDescription::getBenchmarkDescription()` to output extra information customized for our example.

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_eltwiseadd_benchmark_palisade.cpp b_desc print

### 3. Engine initialization

At this point, we can start filling out new information for our specific benchmark inside the engine. This is required to make our benchmark visible to the Test Harness since the engine is responsible for managing the benchmarks' lifespans. First we are going to add options to the `TutorialEngine::init()` method which implements the `hebench::cpp::BaseEngine::init()` virtual method. The order of initialization in the next sections can be any.

Inside of `TutorialEngine::init()` we will see the following code already populated from our default example:

```cpp
void TutorialEngine::init()
{
    // add any new error codes: use
    // this->addErrorCode(code, "generic description");

    // add supported schemes
    addSchemeName(HEBENCH_HE_SCHEME_PLAIN, "Plain");

    // add supported security
    addSecurityName(HEBENCH_HE_SECURITY_NONE, "None");

    // add the all benchmark descriptors
    addBenchmarkDescription(std::make_shared<TutorialEltwiseAddBenchmarkDescription>());
}
```
 
For this tutorial, we will make a few changes as we plan to perform our operations homomorphically.

First, we add any new error codes which we want our engine to be capable of returning back to the frontend. For our example we define one new error as shown below. These errors are defined inside of `tutorial_error_palisade.h`.

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_engine_palisade.cpp engine init error codes

Next, we add the scheme names we want to support. For our simple example we will just add BFV scheme which is predefined as part of `hebench::APIBridge` namespace. Any other schemes can be custom defined as well.

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_engine_palisade.cpp engine init schemes
	
We then add our supported security levels. For our new backend we specify a flag for 128-bit security inside of `tutorial_engine_palisade.h` and then pass it here.

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_engine_palisade.cpp engine init security

Last, in `init()` we add all of the supported benchmarks to the engine. For our example we will be adding just the single example benchmark but as we expand our backend and add more benchmarks, we would add them to this section in the same fashion as shown.

\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_engine_palisade.cpp engine init benchmarks
    
The full code for our engine init function looks like this:
\snippet{lineno} docsrc/examples/backend_tutorials/palisade/tutorial_backend/src/tutorial_engine_palisade.cpp engine init

## Tutorial steps

[Tutorial Home](backend_tutorial_palisade.md)<br/>
[Preparation](backend_tutorial_preparation_palisade.md)<br/>
<b>Engine Initialization and Benchmark Description</b><br/>
[Benchmark Implementation](backend_tutorial_impl_palisade.md)<br/>
[File References](backend_tutorial_files_palisade.md)
