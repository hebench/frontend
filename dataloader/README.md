# CSV Format for Input Datasets 
This format specifies a 3-dimensional tensor object for input parameters and output results. A control line specifies the zero-based first parameter index. Each sample line under the control line becomes the second index. The third index is the individual samples on each line. A control line may specify that the lines below are either 'local' data in the file itself, or a link to an external csv file. In the case of multiple csv files, or multiple control lines for the same index, the lines are concatenated.
## Control line format
```
<input|output>, <operation_input_param_index>, <num_lines>, <local|csv>
<sample_0> 
<sample_1> 
...
<sample_num_lines> 
```
## Example 
Logistic regression - 16 features: 
File name: lr16.csv 
```
# This is just a comment 
input, 0, 1, csv
model.csv, 2, 1 
input, 1, 1, csv
model.csv, 3, 1 
input, 2, 5, local 
-15, 14, -13, 12, -11, 10, -9, 8, -7, 6, -5, 4, -3, 2, -1, 0 
-15, 14, -13, 12, -11, 10, -9, 8, -7, 6, -5, 4, -3, 2, -1, 0 
-15, 14, -13, 12, -11, 10, -9, 8, -7, 6, -5, 4, -3, 2, -1, 0 
-15, 14, -13, 12, -11, 10, -9, 8, -7, 6, -5, 4, -3, 2, -1, 0 
-15, 14, -13, 12, -11, 10, -9, 8, -7, 6, -5, 4, -3, 2, -1, 0 
output, 0, 5, local
0.000911051 
0.000911051 
0.000911051 
0.000911051 
0.000911051 
```
 
File name: model.csv 
```
# Another comment 
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 
1 
```
 
## Comments and blank lines 
Between control line, empty lines are ignored, and lines starting with a pound sign “#” are ignored as comments. There are no in-line comments. Failure on loading operation should throw a C++ exception derived from std::exception. 

##  input
Indicates that the following (non-comment, non-empty) <num_lines> rows in the csv contain the samples for operation input parameter <operation_input_param_index>. As a generic loader, it is responsibility of the file provider to supply the correct number of elements for each input and output sample based on the operation for which the file is going to be used as defined in the HEBench documentation for said operation. 

## output
Providing ground truth values via output is optional during the loading phase. If omitted, the framework will attempt to use the loaded inputs and a workload operation (specified elsewhere) to compute the ground truths outside of the loader.  
If one output sample is specified for an output component in the csv file, then all the possible samples for that component must be specified. 

## <operation_input_param_index>
As a generic loader, it is responsibility of the file provider to supply the correct number of elements for each input and output sample based on the operation for which the file is going to be used as defined in the HEBench documentation for said operation. 

## <num_lines>: 
Indicates that the following (non-comment, non-empty) <num_lines> rows in the csv contain the samples for operation input

## local
local: <sample_i> is a comma-separated list of numbers (doubles). Each element is casted internally to the data type of the input expected for the operation. E.g. 
```
1, 2, 3, 4, 5
``` 
If casting fails, an exception is thrown. 
The size of each sample vector for the input component must match, or an exception is thrown.  
 
## csv 
Loads from a csv filename where each row is a comma-separated list of numbers (doubles) representing a sample. Path can be relative to parent file, or absolute. 
<file_name>,<from_line>, <num_lines> 
### <file_name> 
is the path to the file to load. Its content is assumed to be csv. Path can be relative to parent file, or absolute.  
### <from_line> 
specifies the number of the first line to read from the indicated CSV. If missing, the first line to read from the CSV file is line 1. Line numbering starts at 1. 
### <num_lines> 
specifies the number of lines to read from the indicated CSV file starting at line <from_line>. If missing or 0, the rest of the file is read, starting from line <from_line>. If there are not enough lines in <file_name> to satisfy the specified combination <from_line>, <num_lines>, or <file_name> does not exist, an exception is thrown. The contents of a loaded csv file are lines in the same format as `local` data. 