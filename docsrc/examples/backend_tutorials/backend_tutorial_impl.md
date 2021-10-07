Quickstart C++ Wrapper Backend Tutorial - Benchmark Implementation {#be_tutorial_impl}
========================

The previous steps were all about getting the new backend project, engine, and benchmark description setup to run. Now we need to actually implement our benchmark logic by implementing each of the 7 virtual functions defined in `hebench::cpp::BaseBenchmark` which map to the `hebench::APIBridge` backend interface. For this example we will go over each function and provide an overview of what that function needs to do and the tutorial implemention using SEAL.

For this benchmark example we will be implementing the vector element-wise addition test which is detailed on @ref elementwise_add . Each workload has a detailed description page which provides all information on the number, format and layout of the parameters. A detailed description of the benchmarks algorithm and how each of the previously described parameters are used. And finally the expected format and data layout of the benchmark results.

All the methods that will be called from Test Harness should receive validated inputs; however, it is a good idea to validate those inputs in the case where we are using incompatible versions between Test Harness and our backend, or any other errors occur. For clarity, though, most validation will be omited in this tutorial.

## Steps

### 0. Benchmark Initialization

During construction of our actual benchmark class, we validate the workload flexible parameters that were passed for this benchmark. Afterwards, other benchmark initialization steps are performed. In this case, we are initializing the SEAL engine for our operation.

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark constructor

#### 0.1. Information regarding function communication
Benchmarks are required to implement a number of functions as defined in the APIBridge. These functions are called by the frontend as part of the testing procedure. Each function will recieve some parameters as input, perform some expected operation, and then pass the results back to the frontend which will use the returned results as input to later functions in the flow. This logical flow must be respected by the different functions we have to implement. To see the pipline flow, check @ref function_pipeline_chart .

To enable a high amount of flexibility and enable the widest variety of implementations with the exception of the encode and decode's `hebench::APIBridge::PackedData` parameters, all communication is done via `hebench::APIBridge::Handle objects`. These handle objects are completely opaque to the Test Harness and it is up to the backend to decide what is stored in each handle at each step of the pipeline.

C++ wrapper offers a series of helper methods to ease the creation and data wrapping in these handles. While it is not necessary to use these methods, it is recommended for code correctness, robustness and clarity. See `hebench::cpp::BaseEngine::createHandle()` and `hebench::cpp::BaseEngine::retrieveFromHandle()` for details.

The next sections follow the logical flow order of the function pipeline.

### 1. encode
**hebench::cpp::BaseBenchmark::encode** wraps the `hebench::APIBridge::encode()` function.
Encode receives input for the benchmark in the `const hebench::APIBridge::PackedData *p_parameters` variable.
 
Encode is responsible for rearranging and encoding this data into a format and new memory location that is compatible with the backend.

The encoded data must be compatible with `hebench::cpp::BaseBenchmark::encrypt()` and `hebench::cpp::BaseBenchmark::load()` functions which will recieve the encoding as input in an opaque handle.

<div align="center">
  <img width="700" src="apibridge_encode_diagram.png" /><br>
  <span>API Bridge Encode flow chart.</span>
</div>

For our benchmark, the element-wise add operation has only 2 operands. We have specified in the benchmark description that first is plain text and second is ciphertext. According to documentation, Test Harness will encode all parameters that ought to be encrypted first in a single call to encode, and all the plain text in another call.

First, we retrieve the data pack for the parameter that we will be encoding, then, we create and allocate our internal representation.

This internal representation can be anything we want, as long as we can retrieve it later. We decided to store each encoded sample in a vector because, since this is an **offline** test, each operand has 1 or more samples (2 for first parameter and 5 for the second in particular, based on our benchmark description).

We use a smart pointer to the vector because we want to be able to copy the pointer object later and use the reference counter to avoid leaving dangling. If our internal object does not need to be copied, `shared_ptr` is not really needed.

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark encode allocation

The next step is the actual encoding from the raw data into our internal encoded representation. The raw pointer incoming from Test Harness actually points to an array of `double` in clear text since the data type we described for this benchmark was `Float64`.

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark encode encoding

Finally, we wrap our internal representation in an opaque handle using one of the helper methods for it to cross the API Bridge boundary.

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark encode return

This is the complete listing of our encode method:

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark encode

### 2. encrypt
Encrypt is responsible for recieving the plaintext output from decode and encrypting it into ciphertext.

<div align="center">
  <img width="700" src="apibridge_encrypt_diagram.png" /><br>
  <span>API Bridge Encrypt flow chart.</span>
</div>

Here we retrieve our internal representation from the opaque handle representing the encoded data:

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark encrypt input_handle

Next, we allocate an equivalent internal representation for the encrypted result, and then we encrypt the encoded plain text into our ciphertext before wrapping it into an opaque handle to return through the API Bridge.

This is the complete listing of our encrypt method:

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark encrypt

### 3. load
Load is used to load the cipher texts to an external device for operation. In this tutorial the operations are happening on the same device as we are doing our encryption so we simply forward the input to the next step.

<div align="center">
  <img width="700" src="apibridge_load_diagram.png" /><br>
  <span>API Bridge Encode flow chart.</span>
</div>

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark load

### 4. operate
 Operate is expected to perform the benchmark operation on the provided combination of ciphertexts and cleartexts.

<div align="center">
  <img width="700" src="apibridge_operate_diagram.png" /><br>
  <span>API Bridge Encode flow chart.</span>
</div>
 
 The details of the benchmark algorithm and outputs are described in the workloads documentation in detail. For our example we provide a simple, non optimized implementation for batched vector addition for improved readability.

Operate should, in practice, perform as fast as possible. It should also never return until all the results for all requested operations are available on the remote host or device and ready for retrieval from the local host.

If operate is executing on an external device that supports some sort of data streaming, this can be achieved as follows:

1. Load first chunk of data during loading phase.
2. (in parallel) Operate on current chunk of data.
(in parallel) If more data is available, stream next chunk of data from host into remote.
3. If more data is available, go to ii.
4. Wait for all ongoing operations to complete.
 
\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark operate

 ### 5. store
Store is responsible for copying results back from our device. For our example operate and encrypt/decrypt are occuring on the same device so we just forward the results. 

<div align="center">
  <img width="700" src="apibridge_store_diagram.png" /><br>
  <span>API Bridge Encode flow chart.</span>
</div>

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark store

 ### 6. decrypt
Decrypt recieves result ciphertexts output from store and decrypts them into plaintexts. 

<div align="center">
  <img width="700" src="apibridge_decrypt_diagram.png" /><br>
  <span>API Bridge Encode flow chart.</span>
</div>

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark decrypt

 ### 7 decode
Decode is responsible for recieving encoded result data and writting it back to the output buffer. 

<div align="center">
  <img width="700" src="apibridge_decode_diagram.png" /><br>
  <span>API Bridge Encode flow chart.</span>
</div>

The hebench::APIBridge::PackedData* parameter points to preallocated memory into which the decoded results must be written. The exact size, format, data type, etc. is detailed in the benchmark description which for this example is @ref elementwise_add .

\snippet{lineno} docsrc/examples/backend_tutorials/tutorial_backend/src/tutorial_eltwiseadd_benchmark.cpp benchmark decode

## Tutorial steps

[Tutorial Home](backend_tutorial.md)<br/>
[Preparation](backend_tutorial_preparation.md)<br/>
[Engine Initialization and Benchmark Description](backend_tutorial_init.md)<br/>
<b>Benchmark Implementation</b><br/>
[File References](backend_tutorial_files.md)
