
External Dataset Loader Overview {#dataset_loader_overview}
========================

[Benchmark configuration files](@ref config_file_reference) can specify an external file containing the dataset to use for each benchmark via the `dataset` field. This dataset must contain the input data, and optional ground truths for the benchmark workload operation evaluated on each possible combination of the specified input data (possible combinations are defined in @ref results_order ).

The input and output data must match the workload specifications. This is, the input and output dimensions, the number of elements per input/output sample, and the number of samples, must match what the workload specification requires.

The following are the supported data formats:
- [Format-specific Comma-separated values (CSV)](@ref dataset_csv_reference)

