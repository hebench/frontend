External Dataset CSV Format Reference {#dataset_csv_reference}
========================

[TOC]

The following defines the format for a file of comma-separated values (CSV) specifying the dataset for a benchmark.

A CSV file is loaded and parsed line by line in order from the first line to the last line of the file. The first line is line 1, the top-most line of the file.
  
## Data Block
  
A CSV dataset file contains one or more data blocks. Below is the defined structure of a data block.
  
```csv
<identifier>, <component_index>, <num_samples>, <kind>[, <pad_to_length>[, <pad_value>]]
<sample_0>
<sample_1>
...
<sample_num_samples-1>
```
  
A data block specifies the information for an input or an output along with the data for the samples.
  
A data block starts with a control line followed by an indicated number of samples of the `kind` specified by the control line.
  
### Control Line
The control line specifies the data contained within the data block. It has the following format:

```csv
<identifier>, <component_index>, <num_samples>, <kind>[, <pad_to_length>[, <pad_value>]]
```
  
`<identifier>` - type: `string`: Indicates whether the data block refers to an input or an output for the benchmark operation. This must be one of the following:
- `input`
- `output`
  
`<component_index>` - type: `uint`: Specifies the operation input parameter index or component index for the output. This value is a zero-based index and corresponds to the specific index for the operation input parameter or output component as defined for the particular workload operation. See @ref tests_overview for a list of the supported workloads and their definitions.
  
`<num_samples>` - type: `uint`: Indicates how many data samples are in the data block.
  
`<kind>` - type: `string`: Specifies the type for each sample in the data block. This must be one of the following:
- `local`
- `csv`

`<pad_to_length>` - type: `uint`: Optional argument. Defaults to `0`.

If `<pad_to_length>` is `0`, empty values inside samples will be replaced with `<pad_value>`; otherwise, the CSV dataset loader will attempt to pad all previously read values with `<pad_value>`, until the legnth of data since previous pad is, at least, `<pad_to_length>`. Similarly, if the end of the sample is reached and number of values read is less than `<pad_to_length>`, the loader will perform the pad for the remainder of the sample. See padding example later.

`<pad_value>` - type: (same as the data type of the benchmark). This is the value that the CSV dataset loader will use when padding samples.

### Samples

In a data block, each non-comment, non-empty line is considered a data sample. The structure of the sample is based on the `<kind>` specified in the control line.

These are the possible formats for a sample based on the `<kind>` argument:

- `local`: `<sample_i>` is a comma-separated list of values. Each element is cast internally to the data type of the input expected for the operation.

  Numeric values are read as is into an element of the sample.
  
  String values are read and added as multiple elements where each element is the value of the corresponding byte in the string. Samples can mix numeric and string values.
  
  Values can be enclosed with quotation marks `"` to group a collection of characters in a string. Values in quotations are automatically interpreted as strings. This is useful when the string value contains commas that should not be considered delimiters. Enclosing quotation marks **are not** read as part of the value.
  
  To read a numeric value as a string, a user can enclose it with quotation marks.
  
  Value delimitation when multiple quotations appear inside a pair of opening and closing qotations is not defined.
  

  An example of a `local` sample with 5 elements is:

  ```csv
  1, 2, 3, 4, 5
  ```

  This will be read as `[1, 2, 3, 4, 5]`.
  
  An example of a `local` sample including strings is:

  ```csv
  1, foo, "34", 5
  ```

  This will be read as `[1, 102, 111, 111, 51, 52, 5]`.

  **IMPORTANT**: A local sample is a 1-dimensional vector where each element is a number (strings are converted to numbers, one element per byte). This will be how, once loaded, the data will be laid out in memory. The shape and meaning of the data is given by the benchmark description, configuration file, and the workload definition. For example, in @ref matrix_multiplication , the data for a sample is expected to be laid in memory contiguously, in row major order; so, the local sample vector will be this data memory layout.
  
- `csv`: `<sample_i>` specifies an external CSV file containing data samples. Each data sample in the external CSV file is of type `local`.
  
  The format for a `<sample_i>` is:
  
  ```csv
  <filename>[, <from_line>, <num_samples>]
  ```
  
  where
  
  - `<filename>` - type: `string`: Name of external CSV file to load. Path to file can be relative to parent file, or absolute. If only the filename is supplied, the entire file is used as input.
  
  - `<from_line>` - type: `uint`: Indicates the line number where the data starts in the file.
  
  - `<num_samples>` - type: `uint`: Specifies how many data samples to read from the external CSV file, starting at the specified line number.
  
### Notes on Samples
Please, refer to @ref glossary for terms used in this section.

If multiple data blocks correspond to the same input or output component, the samples in each block will be appended to whatever data already exists for the component, in the order in which they are encountered.

All the samples for the same input or output component must have the same number of elements. Otherwise, an exception is raised when validating the dataset.

The number of samples for each output component must be the same and matches the total number of output samples.

The total number of output samples is the same as the number of input samples. The total number of input samples depends on the number of samples per input component as specified by the rules in @ref results_order .

## Comments and Empty Lines
Blanks at the start and end of a line are ignored.

### Comments
All lines starting with the `#` symbol in CSV files are considered comments and therefore are ignored. There are no inline comments.

### Empty Lines
All empty lines in the CSV files are ignored.

## Padding Samples

Padding is a feature of the CSV dataset loader that allows users to add duplicate values to a sample without the need to explicitly include them. While providing values with manually added padding is perfectly fine, this automatic padding feature exists as a utility.

If a sample contains empty values, the CSV dataset loader will attempt to pad the read data up to the value specified in the control line by `<pad_to_length>` with `<pad_value>`.

For example, see the following data block that specified a `<pad_to_length>` of `4` with a `<pad_value>` of `0`.

```csv
input, 2, 1, local, 4, 0
-15, 14, , 1, 2, 3, 4, , foo, , , -3
```

When read, the resulting sample parsed will be

```
[-15, 14, 0, 0, 1, 2, 3, 4, 102, 111, 111, 0, 0, 0, 0, 0, -3, 0, 0, 0]
        ^----^            ^^             ^-^^----------^    ^-------^
```

Carets were added to signal where padding occurred. Notice that no padding will be added if the read values since last padding match the `<pad_to_length>`.

The same block with `<pad_to_length>` set to default `0` would look a bit different, since the default behavior is to simply replace empty values with `<pad_value>` regardless of padding:

```csv
input, 2, 1, local, 0, 0
-15, 14, , 1, 2, 3, 4, , foo, , , -3
```

When read, the resulting sample parsed will be

```
[-15, 14, 0, 1, 2, 3, 4, 0, 102, 111, 111, 0, 0, -3]
          ^              ^                 ^  ^
```

Carets are marking the empty values converted to `<pad_value>`.

### Common Usage for Padding
A common usage for the automatic padding is converting collection of words into a sample.

For example, assume we have the collection below.

```
Brazil
Canada
Colombia
Mexico
United States
```

We can turn it into a sample with padding as such:

```
input, 1, 1, local, 14, 0
Brazil, , Canada, , Colombia, , Mexico, , United States
```

This will read as a single sample with each word encoded as 14 characters contiguous in memory. Words in the sample that have less than 14 characters will be padded with the specified `<pad_value>` of `0`. Note that in our example, all words are 13 characters or less, so, they will all feature, at least, one padded character.

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
