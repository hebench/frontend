
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

/// @cond

#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

#include "../include/generic_wl_benchmark.h"
#include "../include/generic_wl_engine.h"

//-----------------------------------
// class ExampleBenchmarkDescription
//-----------------------------------

ExampleBenchmarkDescription::ExampleBenchmarkDescription(hebench::APIBridge::Category category)
{
    if (category != hebench::APIBridge::Category::Latency
        && category != hebench::APIBridge::Category::Offline)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid category."), HEBENCH_ECODE_INVALID_ARGS);

    // initialize the descriptor for this benchmark
    std::memset(&m_descriptor, 0, sizeof(hebench::APIBridge::BenchmarkDescriptor));
    m_descriptor.workload  = hebench::APIBridge::Workload::Generic;
    m_descriptor.data_type = hebench::APIBridge::DataType::Float64;
    m_descriptor.category  = category;
    if (category == hebench::APIBridge::Category::Latency)
    {
        m_descriptor.cat_params.min_test_time_ms                = 0;
        m_descriptor.cat_params.latency.warmup_iterations_count = 1;
    } // end if
    m_descriptor.cipher_param_mask = HEBENCH_HE_PARAM_FLAGS_ALL_PLAIN;
    //
    m_descriptor.scheme   = HEBENCH_HE_SCHEME_PLAIN;
    m_descriptor.security = HEBENCH_HE_SECURITY_NONE;
    m_descriptor.other    = 0; // no extras needed for our purpose:
        // Other backends can use this field to differentiate between
        // benchmarks for which internal parameters, not specified by
        // other fields of this structure, differ.

    // specify default arguments for this workload
    hebench::cpp::WorkloadParams::Generic default_workload_params(2, 3);
    default_workload_params.length_InputParam(0)      = 2;
    default_workload_params.length_InputParam(1)      = 2;
    default_workload_params.length_ResultComponent(0) = 2;
    default_workload_params.length_ResultComponent(1) = 2;
    default_workload_params.length_ResultComponent(2) = 1;
    this->addDefaultParameters(default_workload_params);
}

ExampleBenchmarkDescription::~ExampleBenchmarkDescription()
{
    // nothing needed in this example
}

hebench::cpp::BaseBenchmark *ExampleBenchmarkDescription::createBenchmark(hebench::cpp::BaseEngine &engine,
                                                                          const hebench::APIBridge::WorkloadParams *p_params)
{
    if (!p_params)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid empty workload parameters. This workload requires flexible parameters."),
                                         HEBENCH_ECODE_CRITICAL_ERROR);

    ExampleEngine &ex_engine = dynamic_cast<ExampleEngine &>(engine);
    return new ExampleBenchmark(ex_engine, m_descriptor, *p_params);
}

void ExampleBenchmarkDescription::destroyBenchmark(hebench::cpp::BaseBenchmark *p_bench)
{
    // make sure we are destroying a benchmark object we created
    if (p_bench)
    {
        ExampleBenchmark *p_tmp = dynamic_cast<ExampleBenchmark *>(p_bench);
        delete p_tmp;
    } // end if
}

//------------------------
// class ExampleBenchmark
//------------------------

ExampleBenchmark::ExampleBenchmark(ExampleEngine &engine,
                                   const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                   const hebench::APIBridge::WorkloadParams &bench_params) :
    hebench::cpp::BaseBenchmark(engine, bench_desc, bench_params)
{
    // validate workload parameters

    // number of workload parameters
    if (bench_params.count < ExampleBenchmarkDescription::NumWorkloadParams)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid workload parameters. This workload requires " + std::to_string(ExampleBenchmarkDescription::NumWorkloadParams) + " parameters."),
                                         HEBENCH_ECODE_INVALID_ARGS);

    // check values of the workload parameters and make sure they are supported by benchmark:

    hebench::cpp::WorkloadParams::Generic w_params(bench_params);

    if (w_params.n() != ParametersCount
        || w_params.m() != ResultComponentsCount)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid workload parameters for `n` and `m`."),
                                         HEBENCH_ECODE_INVALID_ARGS);
    if (w_params.length_InputParam(0) != 2
        || w_params.length_InputParam(1) != 2)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid workload parameters for `length_InputParam`."),
                                         HEBENCH_ECODE_INVALID_ARGS);
    if (w_params.length_ResultComponent(0) != 2
        || w_params.length_ResultComponent(1) != 2
        || w_params.length_ResultComponent(2) != 1)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid workload parameters for `length_ResultComponent`."),
                                         HEBENCH_ECODE_INVALID_ARGS);
}

ExampleBenchmark::~ExampleBenchmark()
{
    // nothing needed in this example
}

hebench::APIBridge::Handle ExampleBenchmark::encode(const hebench::APIBridge::DataPackCollection *p_parameters)
{
    if (p_parameters->pack_count != ParametersCount)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid number of parameters detected in parameter pack. Expected " + std::to_string(ParametersCount) + "."),
                                         HEBENCH_ECODE_INVALID_ARGS);

    // allocate our internal version of the encoded data

    InternalInputData input_data;

    for (std::size_t param_i = 0; param_i < input_data.size(); ++param_i)
    {
        std::vector<std::vector<double>> &input_param_batch = input_data[param_i];
        const hebench::APIBridge::DataPack &data_pack       = findDataPack(*p_parameters, param_i);
        input_param_batch.resize(data_pack.buffer_count);
        for (std::size_t sample_i = 0; sample_i < input_param_batch.size(); ++sample_i)
        {
            const hebench::APIBridge::NativeDataBuffer &sample_buffer = data_pack.p_buffers[sample_i];
            if (!sample_buffer.p)
                throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid empty sample in input parameter."),
                                                 HEBENCH_ECODE_INVALID_ARGS);
            const double *p_raw_buffer = reinterpret_cast<const double *>(sample_buffer.p);
            input_param_batch[sample_i].assign(p_raw_buffer, p_raw_buffer + (sample_buffer.size / sizeof(double)));
        }
    } // end if

    // wrap our internal object into a handle to cross the boundary of the API Bridge
    return this->getEngine().template createHandle<decltype(input_data)>(sizeof(InternalInputData), 0,
                                                                         std::move(input_data));
}

void ExampleBenchmark::decode(hebench::APIBridge::Handle encoded_data, hebench::APIBridge::DataPackCollection *p_native)
{
    // This method should handle decoding of data encoded using encode(), due to
    // specification stating that encode() and decode() are inverses; as well as
    // handle data decrypted from operation() results.

    // retrieve our internal format object from the handle
    const InternalResultData &encoded =
        this->getEngine().template retrieveFromHandle<InternalResultData>(encoded_data);

    // according to specification, we must decode as much data as possible, where
    // any excess encoded data that won't fit into the pre-allocated native buffer
    // shall be ignored

    std::uint64_t min_component_count = std::min(p_native->pack_count, static_cast<std::uint64_t>(ResultComponentsCount));
    for (std::size_t component_i = 0; component_i < min_component_count; ++component_i)
    {
        hebench::APIBridge::DataPack *p_native_param = &p_native->p_data_packs[component_i];

        if (p_native_param && p_native_param->buffer_count > 0)
        {
            std::uint64_t min_sample_count = std::min(p_native_param->buffer_count, encoded.size());
            for (std::size_t sample_i = 0; sample_i < min_sample_count; ++sample_i)
            {
                // for latency, we have only one sample, so, decode the sample into the first buffer
                hebench::APIBridge::NativeDataBuffer &native_sample = p_native_param->p_buffers[sample_i];
                if (native_sample.p && native_sample.size > 0)
                {
                    std::uint64_t min_size = std::min(native_sample.size / sizeof(double),
                                                      encoded[sample_i][component_i].size());
                    double *p_raw_buffer   = reinterpret_cast<double *>(native_sample.p);
                    std::copy(encoded[sample_i][component_i].begin(), encoded[sample_i][component_i].begin() + min_size,
                              p_raw_buffer);
                } // end if
            } // end for
        } // end if
    } // end for
}

hebench::APIBridge::Handle ExampleBenchmark::encrypt(hebench::APIBridge::Handle encoded_data)
{
    // we only do plain text in this example, so, just return a copy of our internal data
    return this->getEngine().duplicateHandle(encoded_data);
}

hebench::APIBridge::Handle ExampleBenchmark::decrypt(hebench::APIBridge::Handle encrypted_data)
{
    // we only do plain text in this example, so, just return a copy of our internal data
    return this->getEngine().duplicateHandle(encrypted_data);
}
hebench::APIBridge::Handle ExampleBenchmark::load(const hebench::APIBridge::Handle *p_local_data, uint64_t count)
{
    if (count != 1)
        // we do all ops in plain text, so, we should get only one pack of data
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid number of handles. Expected 1."),
                                         HEBENCH_ECODE_INVALID_ARGS);
    if (!p_local_data)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid null array of handles: \"p_local_data\""),
                                         HEBENCH_ECODE_INVALID_ARGS);

    // since remote and host are the same for this example, we just need to return a copy
    // of the local data as remote.

    return this->getEngine().duplicateHandle(p_local_data[0]);
}

void ExampleBenchmark::store(hebench::APIBridge::Handle remote_data,
                             hebench::APIBridge::Handle *p_local_data, std::uint64_t count)
{
    if (count > 0 && !p_local_data)
        throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid null array of handles: \"p_local_data\""),
                                         HEBENCH_ECODE_INVALID_ARGS);

    if (count > 0)
    {
        // pad with zeros any remaining local handles as per specifications
        std::memset(p_local_data, 0, sizeof(hebench::APIBridge::Handle) * count);

        // since remote and host are the same for this example, we just need to return a copy
        // of the remote as local data.

        p_local_data[0] = this->getEngine().duplicateHandle(remote_data);
    } // end if
}

hebench::APIBridge::Handle ExampleBenchmark::operate(hebench::APIBridge::Handle h_remote_packed,
                                                     const hebench::APIBridge::ParameterIndexer *p_param_indexers)
{
    // This method should perform as fast as possible since it is the
    // method benchmarked by Test Harness.

    const InternalInputData &InputParams = this->getEngine().template retrieveFromHandle<InternalInputData>(h_remote_packed);
    assert(InputParams.size() == ParametersCount);

    for (std::size_t i = 0; i < InputParams.size(); ++i)
        // normally, a robust backend will use the indexers as appropriate,
        // but for the sake of the example, we just validate them
        if (p_param_indexers[i].value_index != 0 || p_param_indexers[i].batch_size != InputParams[i].size())
            throw hebench::cpp::HEBenchError(HEBERROR_MSG_CLASS("Invalid parameter indexer. Expected index 0 and batch size of 1."),
                                             HEBENCH_ECODE_INVALID_ARGS);

    // create a new internal object for result
    InternalResultData retval;
    retval.reserve(InputParams[0].size() * InputParams[1].size());

    // perform the actual operation
    hebench::cpp::WorkloadParams::Generic w_params(this->getWorkloadParameters());
    for (std::size_t sample_0_i = 0; sample_0_i < InputParams[0].size(); ++sample_0_i)
    {
        for (std::size_t sample_1_i = 0; sample_1_i < InputParams[1].size(); ++sample_1_i)
        {
            assert(InputParams[0][sample_0_i].size() == w_params.length_InputParam(0));
            assert(InputParams[1][sample_0_i].size() == w_params.length_InputParam(1));
            std::array<std::vector<double>, ResultComponentsCount> ResultComponents;

            // ResultComponent[0] = InputParam[0] + InputParam[1]
            ResultComponents[0].resize(w_params.length_ResultComponent(0));
            std::transform(InputParams[0][sample_0_i].begin(), InputParams[0][sample_0_i].end(),
                           InputParams[1][sample_1_i].begin(), ResultComponents[0].begin(),
                           std::plus<double>());

            // ResultComponent[1] = InputParam[0] - InputParam[1]
            ResultComponents[1].resize(w_params.length_ResultComponent(1));
            std::transform(InputParams[0][sample_0_i].begin(), InputParams[0][sample_0_i].end(),
                           InputParams[1][sample_1_i].begin(), ResultComponents[1].begin(),
                           std::minus<double>());

            // ResultComponent[2] = InputParam[0] . InputParam[1]
            ResultComponents[2].resize(w_params.length_ResultComponent(2));
            ResultComponents[2].front() = std::inner_product(InputParams[0][sample_0_i].begin(), InputParams[0][sample_0_i].end(),
                                                             InputParams[1][sample_1_i].begin(), 0.0);

            retval.emplace_back(std::move(ResultComponents));
        } // end for
    } // end for

    // send our internal result across the boundary of the API Bridge as a handle
    return this->getEngine().template createHandle<decltype(retval)>(sizeof(retval), 0,
                                                                     std::move(retval));
}

/// @endcond
