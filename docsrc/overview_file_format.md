
Run Overview Output Format        {#overview_file_format}
========================

[TOC]

The following details the format and description of the run overview file produced by [Report Compiler](@ref report_compiler_overview).

The overview file is a highlight of the main event of each report processed during the report compiler run. Its main purpose is to summarize and offer an easy way to see the big picture of the run at a glance. It is also intended to simplify parsing of the reports where statistical information for each main event is aggregated and conveniently available for further use. If more details are required from a report, the overview file points to each file from which the data was extracted. On a successful run, statistics and summary files are located in the same location as the report files.

## Overview example output

This example shows an example of the contents of an overview file.

```csv
,,,,,,,,,,,,,,Wall Time,,,,,,,,,,,,,CPU Time
Workload,End State,Filename,Category,Data type,Cipher text,Scheme,Security,Extra,ID,Event,Total Wall Time,Samples per sec,Samples per sec trimmed,Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,Average,Standard Deviation,Time Unit,Time Factor,Min,Max,Median,Trimmed Average,Trimmed Standard Deviation,1-th percentile,10-th percentile,90-th percentile,99-th percentile,Input Samples,wp0,wp1,wp2
Matrix Multiplication (100x100) x (100x100),OK,"/linux-storage/storage/documents/repos/hebench/frontend/build/report/matrix_multiplication_0-wp_100_100_100-latency-float64-20001-all_plain-plain-none-0-report.csv",Latency,Float64,0,Plain,None,0,108,Operation,2019.196494,50.019896676782,50.98381115765,19.992044495049,2.661381013492,ms,0.001,19.537712,45.72633,19.609865,19.614069197531,0.034799752167,19.543096,19.563912,19.71045,20.344244,19.99102970297,2.661467916228,ms,0.001,19.538,45.726,19.609,19.613,0.034760250287,19.543,19.563,19.711,20.343,101,100,100,100
```

## File Contents

This file features a table where each row contains the statistical data compiled for the corresponding report's main event. The table shares many columns with a statistics file.

Here are listed the columns that are unique to the overview file. For details on the other columns, see @ref stats_n_summary_format .

| Column name | Description |
|-|-|
| Workload | Name of the workload as extracted from the header of the report file <BR> or <BR> "Failed" if the corresponding report belongs to a failed benchmark. |
| End State | Shows the completion status per benchmark: whether it completed successfully (`OK`) or failed (`Failed`). <BR> See benchmark report file for cause of failure. |
| Filename | Full path to the report file used to generate these statistics. If the report is not a failed benchmark, the statistics and summary files should be located in the same location as the report after a successful compilation. |
| Category | Testing category <BR> or <BR> Reason for the benchmark failure, if the corresponding report belongs to a failed benchmark. |
| Data type | Data type used for input and output in the test. |
| Cipher text | Binary representation of which operation parameters were encrypted. Each bit position corresponds to the operation parameter index. |
| Scheme | Homomorphic Encryption scheme used. |
| Security | Encryption security level. |
| Extra | Extra identifier of the benchmark according to the report. |

Most fields will be blank if the corresponding report belongs to a failed benchmark.
