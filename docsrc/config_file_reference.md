Benchmark Configuration File Reference {#config_file_reference}
========================

Benchmark configuration files can be specified during a run of Test harness via the `--benchmark_config_file` command line argument.

A configuration file contains a list of benchmarks to run and parameters to use for each workload. While the syntax of configuration files is the same for all, IDs for benchmarks are specific to each backend.

## Configuration file syntax

A configuration file is a YAML file with the following syntax:

```yaml
default_min_test_time: <min_test_time_ms>
default_sample_size: <sample_size>
random_seed: <seed>

benchmark:
  - ID: <benchmark_id>
    params:
      0:
        name: <param_name>
        type: <param_type>
        value:
          from: <value_from>
          to: <value_to>
          step: <value_step>
      1:
        name: <param_name>
        type: <param_type>
        value:
          from: <value_from>
          to: <value_to>
          step: <value_step>
      ...
  - ID: <benchmark_id>
    params:
      0:
        name: <param_name>
        type: <param_type>
        value:
          from: <value_from>
          to: <value_to>
          step: <value_step>
      1:
        name: <param_name>
        type: <param_type>
        value:
          from: <value_from>
          to: <value_to>
          step: <value_step>
      ...
  ...
```

### Benchmark parameters

The configuration file can have settings for certain default behaviors. These are optional.

- `default_min_test_time`: Specifies the default minimum test time in *milliseconds*. If missing, it defaults to `0`.
For **Latency** tests that support flexible test times, this is the minimum time for which they will run. As always, regardless of the value specified, all latency tests will run, at least, two iterations.
**Offline** tests will run through the whole dataset, at least, once. If this minimum test time hasn't elapsed by the end of the run, a new run is executed. This behavior continues until the test time elapses.

- `default_sample_size`: Specifies the number of samples to be used for operation parameters in **Offline** tests that support flexible sample size. Defaults to `0` if missing.
Offline benchmarks can directly specify the sample size for each operation parameter, or indicate which parameters support flexible sample sizes.
All workloads define a default sample size if none is specified for flexible parameters. When this setting is missing or set to `0`, indicates that these pre-defined workload sample sizes are to be used for flexible parameters.

- `random_seed`: type: `uint64`. Specifies the seed for the random number generator to use when generating synthetic data. When missing, the global Test Harness seed will be used (see command line `--random_seed` in @ref test_harness_usage_guide ). This value can be used to replicate results during tests.


### Benchmark descriptions

Top level `benchmark` key contains a list. It must exist in the configuration file. An element of this list specifies a benchmark to run and its description.

A benchmark description is composed by `ID` and `params`.

A benchmark executes a specific workload from the set of workloads specified in hebench::APIBridge::Workload enumeration. A backend implements a collection of benchmarks and registers them with the Front end during initialization.

The value of field `ID` is `<benchmark_id>`. This is an integer number identifying the benchmark to run. These IDs are backend specific that map to a registered benchmark. To obtain the correct ID, users can either find out in the backend documentation, or by exporting the backend default configuration file.

#### Workload parameters

Workloads executed by benchmarks have a number of mandatory parameters. The number and type for these parameters is workload specific. Required parameters for each workload are listed under the [documentation for each workload](@ref tests_overview).

Arguments for the benchmark’s workload parameters are specified under `params`. This is technically a list of parameters. Each argument is identified by its zero-based index. This index must correspond to the one in the workload documentation. Under its index, an argument specifies a `name`, a `type`, and a `value`.

The value of field `name` is `<param_name>`. This is any string used for description purposes. This string can be anything as long as it is unique inside the benchmark section. Names already populated by exported configuration files can be changed, but it is not recommended.

The value of field `type` is `<param_type>`. This is a string and  must be one of `UInt64`, `Int64`, `Float64`. This is not case sensitive. The correct type for a workload parameter is listed in the corresponding workload documentation.

The `value` field specifies a range of values for this argument. The sub-fields `from`, `to` and `step` must be numbers compatibles with the type specified by `<param_type>`. Note that `<value_to>` must be greater than or equal to `<value_from>`.

A `<value_step>` of zero means that only `<value_from>` will be used in the range.

The range of values for the argument will run as follows:

```cpp
if (value_step != 0)
{
    for (value = value_from; value <= value_to; value += value_step)
    {
        // do something with value
    }
}
else
{
    value = value_from;
    // do something with value
}
```

For workloads with multiple parameters, all possible combinations for each range will be generated and the benchmark for each combination executed by Test harness. The order of each combination is undefined.

Finally, backends may have extra workload parameters, beyond those required. Configuration files are expected to fulfill these as well. To know if and which extra parameters a backend has defined for a benchmark, users must consult the specific backend documentation. Exported configuration files may offer a hint at any extra parameters as well.

## Default benchmark configuration

The best starting point for creating a custom benchmark configuration file is to export the default configuration for a backend.

The exported file will contain the correct information regarding each benchmark ID and its workload parameters. Each benchmark will be preceded by a comment describing what the configuration represents (workload, category, category parameters, etc.).

Users may add, edit or remove benchmarks in this file as needed. Invalid configurations will be rejected by Test harness when loading.

For example, if a backend exported configuration looks like below:

```yaml
default_min_test_time: 0
default_sample_size: 0
random_seed: 1679203945

benchmark:

# Benchmark with workload parameters:
#   Logistic Regression PolyD3 16 features
# Descriptor:
#   wp_16 | offline | float64 | 1120 | all_cipher | ckks | 128 | 1
  - ID: 3
    params:
      0:
        name: n
        type: UInt64
        value:
          from: 16
          to: 16
          step: 0
```
we know that `ID` value of `3` will always represent a "Logistic Regression PolyD3" workload and descriptor "offline | float64 | 1120 | all_cipher | ckks | 128 | 1". The number of features `n` is the workload parameter `0` as specified in @ref logistic_regression .

We can modify the parameters at will, as long as our new values are supported by the backend used to export this file.

We can add more benchmarks, as long as their IDs are one of the IDs in the original exported file, and the number of parameters and their types match the correct workload.

Note that adding benchmarks that exactly match parameters of other existing benchmarks will not cause an error. Test harness will run duplicate benchmarks, but the results of the last run will overwrite the results of any previous runs of the duplicated benchmark.

### Exporting default configuration
The following command will make Test harness query the specified backend and generate the file pointed by variable `$CONFIG_FILE_PATH` containing the benchmark configuration information to run the backend with default parameters, instead of running any benchmarks.

If the file already exists, it will be overwritten without notification.

```bash
./test_harness --backend_lib_path $BACKEND_LIB --benchmark_config_file $CONFIG_FILE_PATH --dump_config
```

### Running with a configuration file

The command below will launch the Test harness which will load the file pointed in `$CONFIG_FILE_PATH`, validate that the selection of benchmarks and parameters are supported by the backend, and then execute only those benchmarks specified in the configuration file.

```bash
./test_harness --backend_lib_path $BACKEND_LIB --benchmark_config_file $CONFIG_FILE_PATH
```
