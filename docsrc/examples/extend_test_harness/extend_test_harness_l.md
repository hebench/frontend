Adding Latency Category Using Pre-existing Implementation                {#extend_test_harness_l}
========================

Class `hebench::TestHarness::BenchmarkLatency` offers a default implementation for **Latency** category. If a Latency category with default workflow needs to be added to a workload, clients should inherit from this class.

## Default Latency Workflow

The following pseudo-code depicts the default Latency workflow as implemented by `hebench::TestHarness::BenchmarkLatency::run()`. Descendants of class `hebench::TestHarness::BenchmarkLatency` will inherit this functionality.

Note: all data is cleaned up and handles are released as appropriate. Boiler plate logic has been omited. Exceptions are thrown on errors. `ErrorException` objects are used as exceptions with error numbers from back-end. Test Harness handles these exceptions as appropriate based on the documentation specified for error codes in the [API Bridge](@ref APIBridge_overview).

```cpp
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

// load encrypted data into back-end's remote to use as input to the operation
Handle h_remote_inputs;
load(h_benchmark,
     &local_handles.data(), local_handles.size()
     &h_remote_inputs);

// prepare parameter indexers for operation
ParameterIndexer params[op_params_count];
for (ParameterIndexer &pi : params)
{
    pi.batch_size = 1;  // only 1 value used for latency test
    pi.value_index = 0; // starting with the first sample
}

// do warm-up iterations
repeat for warm-up iterations
{
    Handle h_remote_result;
    startTiming();
    operate(h_benchmark, h_remote_inputs, params, &h_remote_result);
    stopTiming();
}

// operate
vector<Handle> h_remote_results;
repeat for specified time and number of iterations >= 2
{
    Handle h_remote_result;
    startTiming();
    operate(h_benchmark, h_remote_inputs, params, &h_remote_result);
    stopTiming();
    
    h_remote_results.push_back(h_remote_result);
}

// postprocess output

vector<Handle> h_local_results;
for (Handle h_remote_result: h_remote_results)
{
    // retrieve data from back-end's remote and store in host
    Handle h_local_result;
    startTiming();
    store(h_benchmark, h_remote_result, &h_local_result, 1);
    stopTiming();
    h_local_results.push_back(h_local_result);
}

// operation results are expected to be encrypted

// decrypt results
vector<Handle> h_plain_results;
for (i = 0; i < h_local_results.size(); ++i)
{
    Handle h_plain_result;
    startTiming();
    decrypt(h_benchmark, h_local_results[i], &h_plain_result);
    stopTiming();
    
    h_plain_results.push_back(h_plain_result);
}

// decode and validate the results

for (Handle h_plain_result : h_plain_results)
{
    // decode
    PackedData packed_raw_result; // pre-allocated
    startTiming();
    decode(Handle h_benchmark, h_plain_result, &packed_raw_result);
    stopTiming();
    
    // validate output
    validateResult(packed_raw_result);
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

## Implementing Latency Category in a Workload Using BenchmarkLatency

These steps should provide a guide to add a Latency category test to a workload. Assume the workload in question is <b>"My Workload"</b>.

Refer to implementation of `EltwiseAdd` as a good example.

### 0. Create directory structure to house the workload and the category.

Create the directory structure to house "My Workload", if not already there. A good starting point to avoid starting from scratch is to copy an existing workload directory to the `hebench/test_harness/benchmarks` location, such as directory `EltwiseAdd` and rename to `MyWorkload`. Rename all files and inner directories in the new directory to match the workload name.

See [Directory Structure](#directory-structure) section above for more information.

### 1. Inherit from BenchmarkLatency.

Create the `MyWorkloadLatency` class that inherits from `public hebench::TestHarness::BenchmarkLatency`.

To inherit from `hebench::TestHarness::BenchmarkLatency`, include header `hebench_benchmark_latency.h` located in `hebench/test_harness/benchmarks/categories/include`.


### 2. Implement the Benchmark.

#### i. Initialization.

During benchmark creation and initialization the framework will call the methods of a benchmark automatically in the following order:

- `MyWorkloadLatency::MyWorkloadLatency`
- `MyWorkloadLatency::init()`
- `MyWorkloadLatency::postInit()`

##### Method init()

Inherited method `hebench::TestHarness::BenchmarkLatency::init()` is abstract and always must be implemented . This method is provided to perform initialization steps before the back-end itself is initialized. It is a dedicated method to provide the ability of polymorphic initialization if required.

* <b>Create a workload compatible </b> `hebench::TestHarness::IDataLoader`

    At any point, during initialization of the benchmark, an object of type `IDataLoader::Ptr` that is compatible with the workload must be created. It must contain, at least, the input parameters for the operation. For latency category, only one sample per parameter is needed. Ground truth for the operation with the specified samples per parameter is also recommended to be loaded into the `IDataLoader` object.
    
    The task of the data loader is to load input samples and corresponding ground truth for the workload operation. In the case where the data loader generates the input data instead of loading from another source (such as storage), the data loader/generator must also precompute the ground truths for each corresponding input sample generated if ground truths are to be preloaded.
    
    A data loader derived from class `hebench::TestHarness::PartialDataLoader` has a head start on common implementations for trivial interface methods. It also takes care of boiler plate code and memory allocation based on parameter information.
    
    If random data needs to be generated, class `hebench::TestHarness::DataGeneratorHelper` can be helpful.

##### Method postInit()

Virtual method `hebench::TestHarness::BenchmarkLatency::postInit()` allows initialization that requires the back-end to already exist. It provides a default implementation and, unless necessary, an override is not required. If one is provided, however, it is required that the base implementation is called as the first step.
    
#### ii. Override remaining methods.

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

At this point, the `MyWorkloadLatency` class should be completed. Refer to documentation on [adding a workload to the Test Harness](extend_test_harness.md) for the final steps to make Test Harness aware of the new class.