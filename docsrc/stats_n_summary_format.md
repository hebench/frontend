
Statistics and Summary Output Format        {#stats_n_summary_format}
========================

[TOC]

The following details the format and description of the statistics and summary files produced by [Report Compiler](@ref report_compiler_overview).

## Statistics example output

This example shows an example of the contents of a statistics file. A summary file is very similar, except that the columns for the table of values are reduced to a few highlights. The table describing each column in a later section indicates which columns are common to statistics and summary files.

```csv
Specifications,
, Encryption, 
, , Scheme, Plain
, , Security, None
, Extra, 0


, Category, Latency
, , Warmup iterations, 1
, , Minimum test time requested (ms), 2000

, Workload, Matrix Multiplication (100x100) x (100x100)
, , Data type, Float64
, , Encrypted op parameters (index), None
, , M = M0 x M1
, , , Rows, Columns, Batch size
, , M0, 100, 100, 1
, , M1, 100, 100, 1
, , M, 100, 100, 1


Notes


Main event,108,Operation

,,,,,Wall Time,,,,,,,,,,,,,CPU Time
ID,Event,Total Wall Time,Samples per sec,Samples per sec trimmed,Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,Input Samples
102,Initialization,0.0312,32051.282051282044,32051.282051282044,0.0312,0,ms,0.001,0.0312,0.0312,0.0312,0.0312,0,0.0312,0.0312,0.0312,0.0312,0.031,0,ms,0.001,0.031,0.031,0.031,0.031,0,0.031,0.031,0.031,0.031,1
104,Encoding pack 1,0.515293,1940.643478564622,1940.643478564622,0.515293,0,ms,0.001,0.515293,0.515293,0.515293,0.515293,0,0.515293,0.515293,0.515293,0.515293,0.515,0,ms,0.001,0.515,0.515,0.515,0.515,0,0.515,0.515,0.515,0.515,1
106,Loading,0.004573,218674.830527007347,218674.830527007347,0.004573,0,ms,0.001,0.004573,0.004573,0.004573,0.004573,0,0.004573,0.004573,0.004573,0.004573,0.005,0,ms,0.001,0.005,0.005,0.005,0.005,0,0.005,0.005,0.005,0.005,1
107,Warmup,49.636525,20.146454652093,20.146454652093,49.636525,0,ms,0.001,49.636525,49.636525,49.636525,49.636525,0,49.636525,49.636525,49.636525,49.636525,49.636,0,ms,0.001,49.636,49.636,49.636,49.636,0,49.636,49.636,49.636,49.636,1
108,Operation,2019.196494,50.019896676782,50.98381115765,19.992044495049,2.661381013492,ms,0.001,19.537712,45.72633,19.609865,19.614069197531,0.034799752167,19.543096,19.563912,19.71045,20.344244,19.99102970297,2.661467916228,ms,0.001,19.538,45.726,19.609,19.613,0.034760250287,19.543,19.563,19.711,20.343,101
109,Store,0.052048999999,1940479.163886555471,2049128.487964677159,0.000515336634,0.000192030637,ms,0.001,0.000376,0.002198,0.000483,0.000488012346,0.000050958928,0.000381,0.000413,0.000608,0.000668,0.000603960396,0.000549166786,ms,0.001,0,0.003,0.001,0.000604938272,0.000491909858,0,0,0.001,0.001,101
110,Decryption,0.069084,1461988.304102751426,1552021.460065327585,0.000684,0.000261768944,ms,0.001,0.000577,0.003139,0.000645,0.000644320988,0.000035152463,0.000578,0.000596,0.000728,0.000884,0.000722772277,0.000512226744,ms,0.001,0,0.003,0.001,0.00075308642,0.00043390276,0,0,0.001,0.001,101
111,Decoding,3.386153,29827.358657450499,30436.550313869833,0.033526267327,0.003481185525,ms,0.001,0.029845,0.050081,0.032827,0.032855234568,0.001111141432,0.030057,0.030983,0.035046,0.042823,0.033504950495,0.003276045672,ms,0.001,0.03,0.049,0.033,0.03287654321,0.001144363535,0.03,0.031,0.035,0.042,101
```

## Header Section

The header section of a statistics file starts from the beginning of the file until the "Notes" section. It is comprised of some standard information:

```csv
Specifications,
, Encryption, 
, , Scheme, <scheme_name>
, , Security, <security_name>
, Extra, <extra_value>
<extra description>

, Category, <test_category_type>

, Workload, <workload_name>
, , Data type, <input_and_output_data_type>
, , Encrypted op parameters (index), <list_of_indices_for_each_encrypted_op_param>
```

The elements listed above will always appear in a header. Some extra elements adding human readable descriptions may appear interspersed throughout.

## Notes Section

The "Notes" section follows the header and may contain important information regarding the benchmark performed, or it could be blank if no extra information is required. Some extra information may include whether validation was disabled during the execution of the benchmark, for example.

## Main Event Section

During a benchmark run, Test Harness measures timings for every event that occurs during execution. Events are grouped by type because some types may occur more than once. While all events are measured, the focus of each supported workload is on the performance of a specific event. Unless otherwise specified the focus in a workload is centered on the "Operation" event type by default (corresponding to calls to the `hebench::APIBridge::operate()` function ). However, the framework is designed such that workloads may indicate a main event type other than "Operation". The event type which is the focus of a workload is known as the <b>main event</b>.

The main event is workload specific and is part of the workload definition; thus, it cannot be changed. Test Harness takes care of properly executing the main event such that statistics can be collected correctly, such as performing requested number of warmup iterations in a latency test, or establishing a minimum execution time in offline category, etc.

Each event type is uniquely identified within the report by a numeric ID. The main event for the workload executed in the benchmark is indicated in the "Main event" section of the statistics and summary files. This section is a single line containing the ID for the main event, followed by the name of the event type.

```csv
Main event,<event_type_id>,<event_name>
```

In general, the name given to event types is used for informational purposes only and is inconsequential. Only the unique event type ID is important. Each event type corresponds to one of the main functions from the [API Bridge function pipeline](@ref function_pipeline_chart) .

## Data Section

The final section of a statistics or summary file is the table containing the data with information regarding the benchmark executed. This data is the result of the statistical computations based on all the events recorded in the benchmark report.

Test Harness measures both, wall time and CPU time for a benchmark.

- Wall time: real time elapsed from the start to the end of an event.
- CPU time: accumulated time that each CPU spent in processing during an event.

The statistics table contains a section for each: Wall time and CPU time.

The following information is collected for both Wall and CPU time.

| Column name | Appears in Summary |  Description |
|-|-|-|
| Average | Y | Average time that the implementation took to process an input sample. |
| Standard Deviation | Y | Standard deviation from the average. |
| Time Unit | Y | Time unit used to represent all timings for the section. |
| Time Factor | Y | Time factor representing the `time unit` in relation to a second. This is, how many `seconds` are there in a `time unit`. Thus, any timing in the section can be converted from the `time unit` into `seconds` by multiplying by this value. |
| Min | N | For all recorded events of the same type, this is the time it took to complete the shortest event. |
| Max | N | For all recorded events of the same type, this is the time it took to complete the longest event. |
| Median | N | 50th percentile timing. |
| Trimmed Average * | N | Average time that the implementation took to process an input sample, computed from the trimmed input samples dataset. |
| Trimmed Standard Deviation * | N | Standard deviation from the trimmed average. |
| 1-th percentile | N | 1th percentile timing. |
| 10-th percentile | N | 10th percentile timing. |
| 90-th percentile | N | 90th percentile timing. |
| 99-th percentile | N | 99th percentile timing. |

The following information is specific for Wall time only.

| Column name | Appears in Summary |  Description |
|-|-|-|
| Total Wall Time | N | Accumulated total wall time elapsed for all events of the same type. |
| Samples per sec | Y | Number of input samples that the benchmark implementation was able to process per second for the specified event type. |
| Samples per sec trimmed * | N | Number of input samples from the trimmed set that the benchmark implementation was able to process per second for the specified event type. |

The following information appears once and applies to both measurements.

| Column name | Appears in Summary |  Description |
|-|-|-|
| ID | Y | Numeric value identifying the event type. The main event section of the file will indicate one of this values. |
| Event | Y | Name of the event type corresponding to the identifier. |
| Input Samples | Y | Total number of input samples processed by the specified operation during the benchmark run. |

`*` The trimmed input samples dataset is obtained from the complete input samples dataset by discarding the top 10% values and bottom 10% values. So, if a dataset has 100 input samples, the corresponding trimmed dataset has 80 values.
