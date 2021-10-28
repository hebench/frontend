Ordering of Results Based on Input Batch Sizes                {#results_order}
========================

The following defines the ordering of results based on batched input.

It is recommended to refer to @ref glossary for a list of terms used in this document.

## Order of Input Samples in a Batch
Some workload categories in HEBench, such as **offline**, allow processing on a batch of input samples so that backends can optimize execution for an input batch, instead of operating on a single input sample at a time.

Given component batches for the input, Test Harness combines a component sample for each input component to form an input sample. The order of the input samples in a batch given the input component samples follows **row-major ordering**. This means that the index for the most significant operation parameter moves faster when computing the index of an input sample in the ordering, given the indices of all of the component samples that make up said input sample.

In general, let `n` be the number of components in the input vector ```(input_0, input_1, ..., input_n-1)```. If component samples in component batch `i` are ordered from `0` to ```cn[i] - 1```, where ```cn[i]``` is the size for component batch `i`, and ```param[i]``` is the index of the component sample to use for operand ```input_i``` , then, the index `r` for an input sample is computed using the algorithm below.

```cpp
r = param[0];
for (i = 1; i < n; ++i)
    r = param[i] + cn[i] * r;
```

The input batch size is:
```cpp
bacth_size = 0;
for (i = 0; i < n; ++i)
    bacth_size *= cn[i];
```
<br>
<hr>
**Example**

Assume a workload for the ternary operation defined as follows:
```
(result_0, result_1) = op3(input_0, input_1, input_2)
```

Assume that Test Harness will request operation on an input batch, and the input samples are specified using a component batch for each operation parameter.

Let ```cn[i]``` be the batch size for component ```input_i```, and:

```
cn[0] = 2
cn[1] = 1
cn[2] = 3
```

which means that there are `2` samples for ```input_0```, `1` sample for ```input_1```, and `3` samples for ```input_2```.

Given this example's configuration and based on the algorithm for computation of the index order `r` for an input sample listed earlier, the following table shows the order in which Test Harness expects a backend to form the input samples based on the component batches:

| `r` | ```param[0]```  | ```param[1]```  | ```param[2]```  |
|---|---|---|---|
| 0  | 0  | 0  | 0  |
| 1  | 0  | 0  | 1  |
| 2  | 0  | 0  | 2  |
| 3  | 1  | 0  | 0  |
| 4  | 1  | 0  | 1  |
| 5  | 1  | 0  | 2  |

or, unrolling the loop for the computation of `r`:

```cpp
r = param[2] + cn[2] * param[1] + cn[2] * cn[1] * param[0]
```
<hr>
## Order of Results Based on Input Batch Ordering

Because the definition of operation requires a mapping from an input sample to a result vector, there is a result for every input sample in an input batch.

When Test Harness passes an input batch of size `n` to a backend for processing, it is expected that the backend will form the input samples using the order defined in the previous section. Test Harness expects `n` results in return, and, because of the operation definition, the result vector at index `r` must be the output of the operation on the `r`-th input sample.
<hr>

**Example**

Following the previous example, given such configuration, Test Harness expects `6` results because the input batch size is `6`. 

If we have the following components to form an input sample:

```cpp
param[0] = 1
param[1] = 0
param[2] = 1
```

then, the sample index for the input and corresponding result is:

```cpp
r = param[2] + cn[2] * param[1] + cn[2] * cn[1] * param[0]
  = 1 + 3 * 0 + 3 * 1 * 1
  = 4
```
<hr>

## Parameter Indexers in APIBridge operate()
A parameter to `hebench::APIBridge::operate()` is `const hebench::APIBridge::ParameterIndexer *p_param_indexers`. This is an array of parameter indexers with as many elements as parameters for the workload operation. A parameter indexer structure is defined as:

```cpp
struct ParameterIndexer
{
    std::uint64_t value_index;
    std::uint64_t batch_size;
};
```
where `value_index` is the index for the component sample inside the loaded input component dataset where this parameter's component batch starts. Field `batch_size` specifies the number of elements in the component batch, starting from the specified index.

This structure is provided for workload flexibility, but Test Harness default behavior is to load a complete batch per component and set the corresponding `ParameterIndexer::value_index` to `0` and `ParameterIndexer::batch_size` to the loaded number of elements for the component batch.

When a workload or category specifies non-default behavior in this regard, the index of the first input sample is computed using the `value_index` for all `ParameterIndexer`s in this array. However, after decoding, the results of the workload operation must always be stored starting at index `0` regardless; where result at `0` is the result of operating on first input sample; result at `1` corresponds to second input sample, and so on. Test Harness will take into account the starting input offset when validating against ground truth values, thus backends must not offset the results.