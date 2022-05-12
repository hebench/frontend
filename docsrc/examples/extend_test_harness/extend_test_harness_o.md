Adding Offline Category Using Pre-existing Implementation                {#extend_test_harness_o}
========================

[TOC]

Class `hebench::TestHarness::BenchmarkOffline` offers a default implementation for **Offline** category. If a Offline category with default workflow needs to be added to a workload, clients should inherit from this class.

## Default Offline Workflow

The following pseudo-code depicts the default Offline workflow as implemented by `hebench::TestHarness::BenchmarkOffline::run()`. Descendants of class `hebench::TestHarness::BenchmarkOffline` will inherit this functionality.

Note: all data is cleaned up and handles are released as appropriate. Boiler plate logic has been omited. Exceptions are thrown on errors. `ErrorException` objects are used as exceptions with error numbers from backend. Test Harness handles these exceptions as appropriate based on the documentation specified for error codes in the [API Bridge](@ref APIBridge_overview).

```cpp
// each parameter contains a batch of samples for the offline test
PackedData packed_params_plain;
PackedData packed_params_cipher;

vector<Handle> local_handles;

if (encrypted parameters)
{
    // encode the raw parameters data
    Handle h_encoded_inputs_cipher;
    startTiming();
    encode(h_benchmark, &packed_parameters_cipher, &h_encoded_inputs_cipher);
    stopTiming();

    // encrypt the encoded data
    Handle h_cipher_inputs;
    startTiming();
    encrypt(h_benchmark, h_encoded_inputs, &h_cipher_inputs);
    stopTiming();
    
    local_handles.push_back(h_cipher_inputs);
}

if (plaintext parameters)
{
    // encode the raw parameters data
    Handle h_encoded_inputs_plain;
    startTiming();
    encode(h_benchmark, &packed_parameters_plain, &h_encoded_inputs_plain);
    stopTiming();
    
    local_handles.push_back(h_encoded_inputs_plain);
}

assert(!local_handles.empty());

// If benchmark has encrypted parameters:
//   local_handles[0] <=> encrypted data
//   local_handles[1] <=> plaintext data (if plaintext data)
// Else (this is not common since, at least 1 parameter should be encrypted)
//   local_handles[0] <=> plaintext data

// load encrypted data into backend's remote to use as input to the operation
Handle h_remote_inputs;
load(h_benchmark,
     &local_handles.data(), local_handles.size()
     &h_remote_inputs);

// prepare parameter indexers for operation
ParameterIndexer params[op_params_count];
for (param_i = 0; param_i < op_params_count; ++param_i)
{
    params[param_i].batch_size = ...;  // number of samples for param_i
    params[param_i].value_index = 0; // starting with the first sample
}

// operate
Handle h_remote_result;
while (!elapsed(min_test_time))
{
    startTiming();
    operate(h_benchmark, h_remote_inputs, params, &h_remote_result);
    stopTiming();
    if (!last operate)
        destroyHandle(h_remote_result);
}

// postprocess output

// retrieve data from backend's remote and store in host
Handle h_local_result;
startTiming();
store(h_benchmark, h_remote_result, &h_local_result, 1);
stopTiming();

// operation results are expected to be encrypted

// decrypt results
Handle h_plain_result;
startTiming();
decrypt(h_benchmark, h_local_results[i], &h_plain_result);
stopTiming();

// decode
PackedData packed_raw_result; // pre-allocated
startTiming();
decode(Handle h_benchmark, h_plain_result, &packed_raw_result);
stopTiming();
    
// validate output
for (auto result_tuple : packed_raw_result)
	validateResult(result_tuple);
}
```
<br/>

## Directory Structure {#directory-structure}
In the current directory hierarchy, Test Harness workloads are located in `hebench/test_harness/benchmarks`.  If our workload is <b>"My Workload"</b>, the preferred method for structuring the directories is to create a directory in this location called `MyWorkload`. Inside `MyWorkload`, create directories for each workload category (`latency`, `offline`, etc.) supported by "My Workload".

For a directory structure example, look at existing workloads such as Element-wise vector addition `EltwiseAdd`.

```cpp
EltwiseAdd
 |
 --- include
 |
 --- latency
 |   |
 |   --- include
 |   |
 |   --- src
 |
 --- offline
 |   |
 |   --- include
 |   |
 |   --- src
 |
 --- src
```

*A good starting point to extend TestHarness is to copy an existing workload and renaming directories and files to match "My Workload".*

## Implementing Offline Category in a Workload Using BenchmarkOffline

These steps should provide a guide to add a Offline category test to a workload. Assume the workload in question is <b>"My Workload"</b>.

Refer to implementation of `EltwiseAdd` as a good example.

### 0. Create directory structure to house the workload and the category.

Create the directory structure to house "My Workload", if not already there. A good starting point to avoid starting from scratch is to copy an existing workload directory to the `hebench/test_harness/benchmarks` location, such as directory `EltwiseAdd` and rename to `MyWorkload`. Rename all files and inner directories in the new directory to match the workload name.

See [Directory Structure](#directory-structure) section above for more information.

### 1. Inherit from BenchmarkOffline.

Create the `MyWorkloadOffline` class that inherits from `public hebench::TestHarness::BenchmarkOffline`.

To inherit from `hebench::TestHarness::BenchmarkOffline`, include header `hebench_benchmark_offline.h` located in `hebench/test_harness/benchmarks/categories/include`.


### 2. Implement the Benchmark.

#### i. Initialization.

During benchmark creation and initialization the framework will call the methods of a benchmark automatically in the following order:

- `MyWorkloadOffline::MyWorkloadLatency`
- `MyWorkloadOffline::init()`
- `MyWorkloadOffline::postInit()`

##### Method init()

Inherited method `hebench::TestHarness::BenchmarkOffline::init()` is abstract and always must be implemented . This method is provided to perform initialization steps before the backend itself is initialized. It is a dedicated method to provide the ability of polymorphic initialization if required.

* <b>Create a workload compatible </b> `hebench::TestHarness::IDataLoader`

    At any point, during initialization of the benchmark, an object of type `IDataLoader::Ptr` that is compatible with the workload must be created. It must contain, at least, the input parameters for the operation. For latency category, only one sample per parameter is needed. Ground truth for the operation with the specified samples per parameter is also recommended to be loaded into the `IDataLoader` object.
    
    The task of the data loader is to load input samples and corresponding ground truth for the workload operation. In the case where the data loader generates the input data instead of loading from another source (such as storage), the data loader/generator must also precompute the ground truths for each corresponding input sample generated if ground truths are to be preloaded.
    
    A data loader derived from class `hebench::TestHarness::PartialDataLoader` has a head start on common implementations for trivial interface methods. It also takes care of boiler plate code and memory allocation based on parameter information.
    
    If random data needs to be generated, class `hebench::TestHarness::DataGeneratorHelper` can be helpful.

##### Method postInit()

Inherited method `hebench::TestHarness::BenchmarkOffline::postInit()` allows initialization that requires the backend to already exist. It provides a default implementation and, unless necessary, an override is not required. If one is provided, however, it is required that the base implementation is called as the first step.
    
#### ii. Override methods.

Implement the following abstract methods.

`hebench::TestHarness::PartialBenchmarkCategory::getDataset()`

Default implementation for the virtual methods below exist, so, override if their default behavior does not match the current benchmark.

<em>(optional)</em>

Destructor

`hebench::TestHarness::PartialBenchmarkCategory::validateResult()`

`hebench::TestHarness::PartialBenchmarkCategory::logResult()`

`hebench::TestHarness::PartialBenchmarkCategory::getStartEventID()`

See the method descriptions for more information on the expected functionality and default behavior.

<hr>
<br>

At this point, the `MyWorkloadOffline` class should be completed. Refer to documentation on [adding a workload to the Test Harness](extend_test_harness.md) for the final steps to make Test Harness aware of the new class.
