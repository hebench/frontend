External Dataset CSV Format Reference {#dataset_csv_reference}
========================

[TOC]

The following defines the format for a file of comma-separated values (CSV) specifying the dataset for a benchmark.

A CSV file is loaded and parsed line by line in order from the first line to the last line of the file. The first line is line 1, the top-most line of the file.
  
## Data Block
  
A CSV dataset file contains one or more data blocks. Below is the defined structure of a data block.
  
```csv
<input|output>, <component_index>, <num_samples>, <kind>
<sample_0>
<sample_1>
...
<sample_num_samples-1>
```
  
A data block specifies the information for an input or an output along with the data for the samples.
  
A data block starts with a control line followed by an indicated number of samples of the `kind` specified by the control line.
  
### Control Line
The control line specifies the data contained within the data block.
  
`<input|output>` - type: `string`: Indicates whether the data block refers to an input or an output for the benchmark operation.
  
`<component_index>` - type: `uint`: Specifies the operation input parameter index or component index for the output. This value is a zero-based index and corresponds to the specific index for the operation input parameter or output component as defined for the particular workload operation. See @ref tests_overview for a list of the supported workloads and their definitions.
  
`<num_samples>` - type: `uint`: Indicates how many data samples are in the data block.
  
`<kind>` - type: `string`: Specifies the type for each sample in the data block. This must be one of the following:
- `local`
- `csv`

### Samples

In a data block, each non-comment, non-empty line is considered a data sample. The structure of the sample is based on the `<kind>` specified in the control line.

These are the possible formats for a sample based on the `<kind>` argument:

- `local`: `<sample_i>` is a comma-separated list of numbers. Each element is cast internally to the data type of the input expected for the operation. If casting fails, an exception is thrown at load time.

  The size of each sample vector for the input component must match, or an error is thrown.

  An example of a `local` sample with 5 elements is:

  ```csv
  1, 2, 3, 4, 5
  ```

  **IMPORTANT**: A local sample is a 1-dimensional vector where each element is a number. This will be how, once loaded, the data will be laid out in memory. The shape and meaning of the data is given by the benchmark description, configuration file, and the workload definition. For example, in @ref matrix_multiplication , the data for a sample is expected to be laid in memory contiguously, in row major order; so, the local sample vector will be this data memory layout.
  
- `csv`: `<sample_i>` specifies an external CSV file containing data samples. Each data sample in the external CSV file is of type `local`.
  
  The format for a `<sample_i>` is:
  
  ```csv
  <filename>[, <from_line>, <num_samples>]
  ```
  
  where
  
  - `<filename>` - type: `string`: Name of external CSV file to load. Path to file can be relative to parent file, or absolute. If only the filename is supplied, the entire file is used as input.
  
  - `<from_line>` - type: `uint`: Indicates the line number where the data starts in the file.
  
  - `<num_samples>` - type: `uint`: Specifies how many data samples to read from the external CSV file, starting at the specified line number.
  
<br/>
If multiple data blocks correspond to the same input or output component, the data in each block will be appended to whatever data already exists for the component, in the order in which they are encountered.

## Comments and Empty Lines
Blanks at the start and end of a line are ignored.

### Comments
All lines starting with the `#` symbol in CSV files are considered comments and therefore are ignored. There are no inline comments.

### Empty Lines
All empty lines in the CSV files are ignored.

## Example
Main CSV file:
```csv
# File name: lr16.csv
# Logistic regression - 16 features

# weights
input, 0, 1, csv
# in model.csv file, read 1 sample starting from line 3
model.csv, 3, 1 

# bias
input, 1, 1, csv
# in model.csv file, read 1 sample starting from line 4
model.csv, 4, 1 

# inputs
input, 2, 3, local
-15, 14, -13, 12, -11, 10, -9, 8, -7, 6, -5, 4, -3, 2, -1, 0 
-15, 14, -13, 12, -11, 10, -9, 8, -7, 6, -5, 4, -3, 2, -1, 0 
-15, 14, -13, 12, -11, 10, -9, 8, -7, 6, -5, 4, -3, 2, -1, 0 

# these will be appended to previous data in input 2
input, 2, 2, local
-15, 14, -13, 12, -11, 10, -9, 8, -7, 6, -5, 4, -3, 2, -1, 0 
-15, 14, -13, 12, -11, 10, -9, 8, -7, 6, -5, 4, -3, 2, -1, 0 

# ground truths
output, 0, 5, local
# there are 5 ground truth samples because we have 5 input samples
0.000911051 
0.000911051 
0.000911051 
0.000911051 
0.000911051 
```
External CSV files:

```csv
# File name: model.csv 
# model features/weights 
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 
# bias
1
```

<hr/>
See @ref dataset_loader_overview for more information.
