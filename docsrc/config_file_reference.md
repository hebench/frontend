Benchmark Configuration File Reference {#config_file_reference}
========================

[TOC]

Benchmark configuration files can be specified during a run of Test Harness via the `--benchmark_config_file` command line argument.

A configuration file contains a list of benchmarks to run and parameters to use for each workload. While the syntax of configuration files is the same for all, IDs for benchmarks are specific to each backend.

## Configuration file syntax

A configuration file is a YAML file with the following syntax:

```yaml
default_min_test_time: <fallback_min_test_time_ms>
default_sample_size: <fallback_sample_size>
random_seed: <seed>
initialization_data: <data>

benchmark:
  - ID: <benchmark_id>
    description:
      workload_id: <workload_id>
      workload_name: <name>
      data_type: <type_name>
      category: <category_name>
      scheme: <scheme_name>
      security: <security_name>
      cipher_flags: <ciphertext/plaintext_op_params>
      other: <descriptor_extra_flags>
      notes: <benchmark_notes>
    dataset: <file_name>
    default_min_test_time: <min_test_time_ms>
    default_sample_sizes:
      0: <sample_size>
      1: <sample_size>
      ...
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
    description:
      workload_id: <workload_id>
      workload_name: <name>
      data_type: <type_name>
      category: <category_name>
      scheme: <scheme_name>
      security: <security_name>
      cipher_flags: <ciphertext/plaintext_op_params>
      other: <descriptor_extra_flags>
      notes: <benchmark_notes>
    dataset: <file_name>
    default_min_test_time: <min_test_time_ms>
    default_sample_sizes:
      0: <sample_size>
      1: <sample_size>
      ...
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

**Note**: Nulls in YAML can be expressed with keyword `null` or `~` symbol.

### Using Environment Variables as Values

If the value of a key is of primitive type (numbers or strings), then, it can be specified locally by a literal in the configuration file, or externally, via environment variables.

The syntax for an environment variable is the `$` symbol followed by the name of the variable.

When loading the configuration file, Test Harness will substitute all environment variables used by the corresponding environment values (or empty string if the variable is not defined). Errors for incorrect values and types will be reported.

For example, the type of value `<seed>` for key `random_seed` is primitive type `uint64`, so, it can be specified locally with a literal as such:

```yaml
random_seed: 1234
```

or via an environment variable:

```yaml
random_seed: $RND_SEED
```

This should allow users to easily run several configurations without the need to modify their configuration files.

**IMPORTANT**: Mixing environment variables with literals is not supported. A value must be an environment variable or a literal. The whole value will be considered the name of an environment variable if it starts with the `$` symbol.

For example, environment variables as part of a path will not be evaluated, unless the whole path is an environment variable.

```yaml
dataset: /tmp/$MY_FILE
```

In this case, the value for `dataset` will be set to `"/tmp/$MY_FILE"`.

However, if `MY_FILE=/tmp/data.csv` in the execution environment, then, the following...

```yaml
dataset: $MY_FILE
```

results in the value for `dataset` to be set to `"/tmp/data.csv"`.

### Configuration Scope

The actual value used for a benchmark configuration is based on where it is specified first in the priority list. The priority is:

1. Backend specified.
2. Benchmark description in configuration file.
3. Global in configuration file.
4. [Workload definition](@ref tests_overview).
5. Global execution specification (if any).

This means, for example, if a benchmark description defines value `default_min_test_time: 10000`, this will override the configuration file global value for `default_min_test_time` (and any other value down the list).

If any of these is set to default or is missing, then the next down the list is used.

### Global Configuration Description

The configuration file can have settings for certain default behaviors. These are optional.

#### Field default_min_test_time

`default_min_test_time` - type: `uint64`. Specifies the default minimum test time in *milliseconds* for a benchmark.

For **Latency** tests that support flexible test times, this is the minimum time for which they will run. As always, regardless of the value specified, all latency tests will run, at least, two iterations. **Offline** tests will run through the whole dataset, at least, once. If this minimum test time hasn't elapsed by the end of the run, a new run is executed. This behavior continues until the test time elapses.

If missing the global execution default is `0`.

#### Field default_sample_size

`default_sample_size` - type: `uint64`. Specifies the number of samples to be used for an operation parameter in **Offline** tests that support flexible sample size. Inside the benchmark description, this setting specifies the sample size for an operand in the workload operation by index (missing indices, or values of `0` will cause the configuration to use this global configuration file value as a fallback).

A backend can directly specify the sample size for each operation parameter per benchmark for an Offline test using the hebench::APIBridge::CategoryParams in the hebench::APIBridge::BenchmarkDescriptor structure. If the backend sets the sample size for an operation parameter to `0`, it indicates that the parameter supports flexible sample sizes given through a configuration file via `default_sample_size`.
    
When this setting is missing or set to `0` in the configuration file, this indicates that the sample sizes pre-defined in the workload specification are to be used for parameters supporting flexible sample sizes. See @ref tests_overview for the specifications of all supported workloads.

#### Field random_seed

`random_seed` - type: `uint64`. Specifies the seed for the random number generator to use when generating synthetic data. When missing, the global Test Harness seed will be used (see command line `--random_seed` in @ref test_harness_usage_guide ). This value can be used to replicate results during tests.

#### Field initialization_data

`initialization_data` - type: `string`. (Optional field; can be `null`) Contains data to be passed to backend engine during initialization.

If the value of this field is an existing file name, Test Harness will read the file into memory as binary and pass the contents to the backend engine initialization through the call to `initEngine()`. The file name can be relative to the configuration file, or absolute.

If this field does not contain a file name, Test Harness will just forward the specified string as is (no null terminator will be appended).

If this field is null, missing, or contains an empty string, Test Harness will pass `null` values for data to the engine initialization.

All data resulting from this field is kept in Test Harness memory during initialization as an array of bytes. Memory is freed and all pointers to the data become invalid after `initEngine()` call returns.

### Benchmark Configuration Section

Top level `benchmark` key contains a list. This key must exist in the configuration file. An element of the value list specifies a benchmark to run and the corresponding configuration.

A benchmark configuration is composed by `ID`, `dataset`, `default_min_test_time`, `default_sample_sizes` and `params`.

The `description` section for each benchmark is automatically generated when exporting a configuration file for informational purposes only. It is intended to inform which is the benchmark descriptor matching the `benchmark ID`. Its contents are ignored by Test Harness, and thus, providing it is optional and may be omitted.

A benchmark executes a specific workload from the set of workloads specified in hebench::APIBridge::Workload enumeration. A backend implements a collection of benchmarks and registers them with the Frontend during initialization.

#### Field ID

The value of field `ID` is `<benchmark_id>`. This is an integer number identifying the benchmark to run. These IDs are backend specific that map to a registered benchmark. To obtain the correct ID, users can either find out by exporting the backend default configuration file, or, if available, in the backend documentation.

#### Field dataset

The `dataset` field is optional and `<file_name>` is a string specifying a file containing the input data and optional ground truths to use for the benchmark. If this field is a relative path, it is considered relative to the configuration file location. If this field is missing, or the value is `null`, Test Harness will attempt to pre-generate the data as specified in the benchmark's workload definition. Note that some workloads may not support pre-generating data, while others may not support external datasets. See @ref tests_overview for more information on each particular workload.

For formats supported by the Test Harness dataset loader see @ref dataset_loader_overview .

#### Fields default_min_test_time and default_sample_sizes

If `default_min_test_time`, or `default_sample_sizes` are specified, their values override those from the global configuration as per **Configuration Scope** above.

#### Workload parameters

Workloads executed by benchmarks have a number of mandatory parameters. The number and type for these parameters is workload specific. Required parameters for each workload are listed under the [documentation for each workload](@ref tests_overview).

Arguments for the benchmark\u2019s workload parameters are specified under `params`. This is technically a list of parameters. Each argument is identified by its zero-based index. This index must correspond to the one in the workload documentation. Under its index, an argument specifies a `name`, a `type`, and a `value`.

The value of field `name` is `<param_name>`. This is any string used for description purposes. This string can be anything as long as it is unique inside the benchmark section. Names already populated by exported configuration files can be changed, but it is not recommended.

The value of field `type` is `<param_type>`. This is a string describing the type for the workload parameter. It is not case sensitive and must be one of:

  - `UInt64`
  - `Int64`
  - `Float64`

The correct type for each workload parameter is listed in the corresponding workload documentation. Types already populated by exported configuration files must not be changed as they already contain the correct value.

The `value` field specifies a range of values for this argument. The values of sub-fields `from`, `to` and `step` must be numbers compatible with the type specified by `<param_type>` in field `type`. Note that `<value_to>` must be greater than or equal to `<value_from>`.

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

All possible configurations will be generated combining each value in the range specified using `from`, `to`, `step` for each workload parameter. The benchmark for each configuration will be executed by Test Harness. The order of each combination is undefined.

**IMPORTANT**: Because an external dataset provides inputs and outputs for a single benchmark configuration, if a benchmark description specifies an external dataset using the `dataset` field, then, the range of values for every workload parameter of said benchmark must contain a single element. Otherwise, an error is produced.

<br/>
Finally, backends may have extra workload parameters, beyond those required. Configuration files are expected to fulfill these as well. To know if and which extra parameters a backend has defined for a benchmark, users must consult the specific backend documentation. Exported configuration files may offer a hint at any extra parameters as well.

## Default benchmark configuration

The best starting point for creating a custom benchmark configuration file is to export the default configuration for a backend.

The exported file will contain the correct information regarding each benchmark ID and its workload parameters. Each benchmark will be preceded by a comment describing what the configuration represents (workload, category, category parameters, etc.).

Users may add, edit or remove benchmarks in this file as needed. Invalid configurations will be rejected by Test Harness when loading.

For example, if a backend exported configuration looks like below:

```yaml
default_min_test_time: 0
default_sample_size: 0
random_seed: 0
initialization_data: ~

benchmark:

# Section "description" is for informational purposes only. It shows the
# benchmark descriptor matching the benchmark ID. Changing contents of
# "description" has no effect.
  - ID: 3
    description:
      workload_id: 6
      workload_name: Logistic Regression PolyD3
      data_type: Float64
      category: Offline
      scheme: CKKS
      security: 128 bits
      cipher_flags: all_cipher
      other: 0
      notes: ~      
    dataset: ~
    default_min_test_time: 0
    default_sample_size:
      0: 0
      1: 0
      2: 100
    params:
      0:
        name: n
        type: UInt64
        value:
          from: 16
          to: 16
          step: 0
```

we know that for this backend, `ID` value of `3` will always represent what is contained in the description section: a "Logistic Regression PolyD3" workload, in the "Offline" category, with input and output data type "Float64", scheme "CKKS" with "128 bits" security, all operation parameters are encrypted, and the extra flags ("other") has a value of `0`. See @ref logistic_regression for full details on workload specification.

We can modify the parameters at will, as long as our new values are supported by the backend used to export this file.

We can add more benchmarks, as long as their IDs are one of the IDs in the original exported file, and the number of parameters and their types match the correct workload.

Note that adding benchmarks that exactly match parameters of other existing benchmarks will not cause an error. Test Harness will run duplicate benchmarks, but the results of the last run will overwrite the results of any previous runs of the duplicated benchmark.

### Exporting default configuration
The following command will make Test Harness query the specified backend and generate the file pointed by variable `$CONFIG_FILE_PATH` containing the benchmark configuration information to run the backend with default parameters, instead of running any benchmarks.

If the file already exists, it will be overwritten without notification.

```bash
./test_harness --backend_lib_path $BACKEND_LIB --benchmark_config_file $CONFIG_FILE_PATH --dump_config
```

### Running with a configuration file

The command below will launch the Test Harness which will load the file pointed in `$CONFIG_FILE_PATH`, validate that the selection of benchmarks and parameters are supported by the backend, and then execute only those benchmarks specified in the configuration file.

```bash
./test_harness --backend_lib_path $BACKEND_LIB --benchmark_config_file $CONFIG_FILE_PATH
```

See @ref test_harness_usage_guide for more information on using the Test Harness.

