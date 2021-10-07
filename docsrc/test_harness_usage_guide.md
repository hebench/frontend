Test Harness User Guide        {#test_harness_usage_guide}
========================

## Test Harness Usage Overview
This page describes and provides information on how to use the test harness application to configure and run workloads, output reports, load different dynamic backends, and available command line options. For additional test harness information including how to extend it please refer to [Test Harness Overview](@ref test_harness_overview)

## Test Harness Usage
The syntax for using the test_harness is shown below. It consists of calling the executable and a space seperated list of options and arguments. The available options are listed in the options table on this page as well as available through the test_harness help option.
```bash
Usage:
	./test_harness OPTIONS
```
## 1. Test Harness Command Line Options

To display the Test Harness help which lists all the available command line options, use the following command:

```bash
./test_harness -h
```

#### Backend selection and benchmark configuration options
|<div style="width:390px">Option</div>                     | Required|Description|
|---------------------------|--|--------------|
| `--backend_lib_path  <path_to_shared_lib>` <BR> `--backend` <BR> `-b` |Y| Path to backend shared library. The library file must exist and be accessible for reading. |
|  ` --benchmark_config_file <path_to_config_file>` <BR> ``--config_file `` <BR>`` -c `` | N | Path to benchmark run configuration file. YAML file specifying the selection of benchmarks and their workload parameters to run. If not present, all back-end benchmarks will be run with default parameters. |
|``--dump_config`` <BR> ``--dump`` | N | If specified, Test Harness will dump a general configuration file with the possible benchmarks that the back-end can run. This file can be used as starting point template for a benchmark run configuration file. The destination file is specified by ``--benchmark_config_file`` argument. |

#### Report options
|<div style="width:390px">Option</div>                     | Required | Description|
|---------------------------|--|--------------|
| `--enable_validation <bool: 0;false;1;true>` <BR> ``--validation`` | N | Specifies whether results from benchmarks ran will be validated against ground truth. Defaults to "TRUE". |
| ``--report_delay <delay_in_ms>`` | N | Delay between progress reports. Before each benchmark starts, Test Harness will pause for this specified number of milliseconds. Pass 0 to avoid delays. Defaults to 1000 ms.|
| ``--report_root_path <path_to_directory>`` <BR> `--output_dir` | N | Directory where to store the report output files. Directory must exist and be accessible for writing. A directory structure will be generated and any existing files with the same name will be overwritten. Defaults to current working directory "." |
|``--run_overview <bool: 0;false;1;true>`` | N | Specifies whether final summary overview of the benchmarks ran will be printed in standard output (TRUE) or not (FALSE). Results of the run will always be saved to storage regardless. Defaults to "TRUE". |

#### Global default

|<div style="width:390px">Option</div>                     | Required | Description|
|---------------------------|--|--------------|
| `--random_seed <uint64>` <BR> `--seed` | N | Specifies the random seed to use for pseudo-random number generation when none is specified by a benchmark configuration file. If no seed is specified, the current system clock time will be used as seed. |

#### Miscellaneous

|<div style="width:390px">Option</div>                     | Required | Description|
|---------------------------|--|--------------|
| `--version` | N | When present, outputs Test Harness version, required API Bridge version and currently linked API Bridge version. Application exists after this. |
| `-h, /h, \h, --help, /help, \help` | N | Shows this help. Application exists after this. |

## 2. Execute All Default Benchmarks for a Backend
A HEBench backend is a shared library (.so in Linux, .dll in Windows) that exposes the API Bridge functionality ( @ref APIBridge_overview ).

To run all of the tests with default parameters currently supported by a particular backend, call the test harness while also providing the name and location of the backend using the `--backend_lib_path` flag. In the example shown, we are calling test_harness for a backend as if it were located in the same path as test_harness.
```bash
./test_harness --backend_lib_path libmy_backend.so
```
Optionally the output location for the performance report can also be specified. 
```bash
./test_harness --backend_lib_path libmy_backend.so --report_root_path ~/reports/
```

## 3. Configuring Benchmark Execution for a Backend

### 3.1 Running with a benchmark configuration file

Backends can support several workloads and offer a set of default parameters for each workload. Sometimes, however, we may want to run specific workloads, or workloads with parameters other than the default sets.

Test Harness offers facilities to configure the run using a configuration file. This benchmark configuration file is a YAML file that contains the list of which benchmarks to run and the workload parameters to use for each benchmark.

If we already have a configuration file named `config.yaml`, we can load it for a Test Harness run as follows:

```bash
./test_harness --backend_lib_path libmy_backend.so --benchmark_config_file config.yaml
```

This will launch the Test Harness which will load `config.yaml`, validate that the selection of benchmarks and parameters are supported by the backend, and then execute only those benchmarks specified in the configuration file.

### 3.2 Exporting a benchmark configuration file

While the format of a benchmark configuration YAML file is standard for the Test Harness, the benchmark parameters and IDs may differ by backend. So, in order to find out the correct values for the configuration, a user may export the default benchmark configuration for a backend using the following:

```bash
./test_harness --backend_lib_path libmy_backend.so --benchmark_config_file config.yaml --dump_config
```

This will launch the Test Harness which, instead of running any benchmarks, will query the specified backend and generate the file `config.yaml` containing the benchmark configuration information to run the backend with default parameters. This file will contain comments on how to use and what  benchmark is represented by each section.

Exported default configuration files are the starting point for users to create their own run configuration by editing these files to match their needs. Some workload parameter values may not be supported by certain backends, so, it is recommended to consult specific backend documentation for information on supported values.

### 3.3 Configuration file format
For reference on the configuration file format, see @ref config_file_reference