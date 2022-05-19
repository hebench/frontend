Glossary                {#glossary}
========================

[TOC]

## Benchmark

A **benchmark** in HEBench is defined as the instantiation and execution of a *workload*. In a benchmark, values for all the *workload parameters* are specified, and *input dataset* for the operation input vector is given.

### Benchmark Description
A benchmark test is defined by a Workload, a Category for said workload, a scheme for such Category, which inputs are plain text of cipher text, the security level for the scheme, and an extra parameter that is backend specific. A specific set of values for all these components describes a benchmark.

### Category
Defines the type of measuring methodology for a benchmark. For example, *offline* category measures throughput of a workload, as in how many operations per second can the implementation of the workload perform.

### Category Parameter
A placeholder for a value that affects behavior of a category. For example, *number of warm-up iterations* is a parameter for *latency* category.

All category parameters must be specified in a benchmark description.

### Ciphertext
Encrypted data. The ideal encrypted data cannot be accessed without decryption into plaintext first.

### Cleartext
Unencrypted data in native format for the underlying platform. In general, this data is held in memory in a format compatible with primitive C data types (`int`, `long`, `float`, `double`, etc.)

### Plaintext
Encoded cleartext.

### Raw data
Same as **Cleartext**.

## Operation

An **operation** in HEBench is defined as a mapping of an **input** vector with `n` heterogeneous **components** (type of each component may be different) to a **result** vector of `m` *heterogeneous* components, where `m` and `n` are specifically defined for each workload operation.

```
(result_0, result_1, ..., result_m-1) = op(input_0, input_1, ..., input_n-1)
```

#### Example

Assume the operation is scalar addition:

```
(result_0) = op+(input_0, input_1)
```

defined as

```
op+(input_0, input_1) = (input_0 + input_1) = (result_0)
```

and a concrete instantiation:

```
(5) = op+(2, 3)
```

In this case, the input vector has `2` **components**. First  **operand** is `input_0`. Second **operand** is `input_1`. In the instantiation, `(2, 3)` is an input **sample**, `2` and `3` are the **arguments** corresponding to first and second **operation parameters** respectively.

The result vector has 1 **component**: `result_0` is the first **component** of the result vector. `(5)` is a result **sample** where `5` is a **component sample** of the result sample.
<hr>

By these definitions, the **component batch size** for each **component** in the input vector can differ. The **batch size** is the multiplication of the **component batch size** for all **components** of the input vector.

By the definition of **operation**, in contrast to the component batch sizes in the input, the number of **samples** per component of the result vector is the same, and it equals the **batch size**.

### Argument
A **component sample** for the input vector.

### Batch
A collection of samples for the input vector.

### Batch size
Number of samples in a batch.

### Component Batch
A collection of samples for a component of a vector.

### Component Batch Size
Number of component samples in a component batch.

### Component Sample
A concrete value of a component of either input or result vectors.

### Dataset
A collection of input samples for an operation.

### Input Dataset
Same as **Dataset**.

### Input Batch
A subset of samples from the input dataset (with repeating elements allowed).

### Operand
A component of the input vector.

### Operation Parameter
Same as **Operand**.

### Sample
A concrete value of either input or result vectors.

## Workload

A **workload** in HEBench is defined as an operation, along with the dataset of inputs for the operation, parameters, and other specifications for all the workload components.

### Workload Parameter
Flexible placeholder for a value that affects the behavior of a workload.

