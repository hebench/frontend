
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <bitset>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "hebench/modules/timer/include/timer.h"

#include "hebench/api_bridge/api.h"
#include "hebench/modules/general/include/hebench_math_utils.h"
#include "include/hebench_engine.h"

#include "../include/hebench_genericwl_l.h"

namespace hebench {
namespace TestHarness {
namespace GenericWL {
namespace Latency {

//----------------------------
// class BenchmarkDescription
//----------------------------

bool BenchmarkDescriptor::m_b_registered = // register the benchmark with the factory
    hebench::TestHarness::BenchmarkFactory::registerSupportedBenchmark(std::make_shared<BenchmarkDescriptor>());

bool BenchmarkDescriptor::matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                                   const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const
{
    assert(m_b_registered);

    // return true if benchmark is supported

    bool retval =
        BenchmarkDescriptorCategory::matchBenchmarkDescriptor(bench_desc, w_params)
        && (bench_desc.category == hebench::APIBridge::Category::Latency);

    return retval;
}

void BenchmarkDescriptor::completeWorkloadDescription(WorkloadDescriptionOutput &output,
                                                      const Engine &engine,
                                                      const BenchmarkDescription::Backend &backend_desc,
                                                      const BenchmarkDescription::Configuration &config) const
{
    // finish describing workload
    assert(DefaultBatchSize == 1);

    BenchmarkDescriptorCategory::completeWorkloadDescription(output, engine, backend_desc, config);

    auto op_info                                   = fetchIOVectorSizes(config.w_params);
    const std::vector<std::uint64_t> &input_sizes  = op_info.first;
    const std::vector<std::uint64_t> &output_sizes = op_info.second;

    assert(input_sizes.size() == output.operation_params_count);

    // finish benchmark header description

    std::stringstream ss;
    std::vector<uint64_t> batch_sizes(output.operation_params_count);

    std::uint64_t result_batch_size = 1;
    for (std::size_t param_i = 0; param_i < batch_sizes.size(); ++param_i)
    {
        batch_sizes[param_i] = DefaultBatchSize;
        result_batch_size *= batch_sizes[param_i];
    } // end for
    // complete header with workload specifics
    ss << ", , (C[0]";
    for (std::size_t i = 1; i < output_sizes.size(); ++i)
        ss << "; C[" << i << "]";
    ss << ") = op(V[0]";
    for (std::size_t i = 1; i < input_sizes.size(); ++i)
        ss << "; V[" << i << "]";
    ss << ")" << std::endl
       << ", , , Elements, Batch size" << std::endl;
    for (std::size_t i = 0; i < output.operation_params_count; ++i)
    {
        ss << ", , V[" << i << "], " << input_sizes[i] << ", " << batch_sizes[i] << std::endl;
    } // end for
    for (std::size_t i = 0; i < output_sizes.size(); ++i)
    {
        ss << ", , C[" << i << "], " << output_sizes[i] << ", " << result_batch_size << std::endl;
    } // end for

    output.workload_header = ss.str();
}

hebench::TestHarness::PartialBenchmark *BenchmarkDescriptor::createBenchmark(std::shared_ptr<Engine> p_engine,
                                                                             const DescriptionToken &description_token)
{
    assert(m_b_registered);
    Benchmark *retval = nullptr;

    try
    {
        retval = new Benchmark(p_engine, description_token);
    }
    catch (...)
    {
        if (retval)
            delete retval;
        throw;
    }

    return retval;
}

void BenchmarkDescriptor::destroyBenchmark(PartialBenchmark *p_bench)
{
    assert(m_b_registered);
    if (p_bench)
        delete p_bench;
}

//-----------------
// class Benchmark
//-----------------

Benchmark::Benchmark(std::shared_ptr<Engine> p_engine,
                     const IBenchmarkDescriptor::DescriptionToken &description_token) :
    BenchmarkLatency(p_engine, description_token),
    m_op_input_count(0),
    m_op_output_count(0)
{
}

void Benchmark::init()
{
    hebench::Common::EventTimer timer;
    hebench::Common::TimingReportEvent::Ptr p_timing_event;
    std::vector<std::uint64_t> batch_sizes;
    std::stringstream ss;
    auto op_info                                   = BenchmarkDescriptor::fetchIOVectorSizes(this->getBenchmarkConfiguration().w_params);
    const std::vector<std::uint64_t> &input_sizes  = op_info.first;
    const std::vector<std::uint64_t> &output_sizes = op_info.second;

    m_op_input_count  = input_sizes.size();
    m_op_output_count = output_sizes.size();
    batch_sizes.resize(m_op_input_count);

    for (std::size_t param_i = 0; param_i < batch_sizes.size(); ++param_i)
        batch_sizes[param_i] = BenchmarkDescriptor::DefaultBatchSize;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Preparing workload.") << std::endl;

    if (this->getBenchmarkConfiguration().dataset_filename.empty())
        // cannot generate data for generic workload
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Dataset file is required for generic workload benchmarking, but none was provided."));

    ss = std::stringstream();
    ss << "Loading data from external dataset: " << std::endl
       << "\"" << this->getBenchmarkConfiguration().dataset_filename << "\"";
    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
    timer.start();
    // load vectors for input and ground truth from file
    m_data         = DataLoader::create(input_sizes,
                                batch_sizes,
                                output_sizes,
                                this->getBackendDescription().descriptor.data_type,
                                this->getBenchmarkConfiguration().dataset_filename);
    p_timing_event = timer.stop<std::milli>();

    ss = std::stringstream();
    ss << "Total data loaded: " << m_data->getTotalDataLoaded() << " bytes";
    std::cout << IOS_MSG_DONE << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
    ss = std::stringstream();
    ss << "Elapsed wall time: " << p_timing_event->elapsedWallTime<std::milli>() << " ms";
    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
    ss = std::stringstream();
    ss << "Elapsed CPU time: " << p_timing_event->elapsedCPUTime<std::milli>() << " ms";
    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
}

bool Benchmark::validateResult(IDataLoader::Ptr dataset,
                               const std::uint64_t *param_data_pack_indices,
                               const std::vector<hebench::APIBridge::NativeDataBuffer *> &outputs,
                               hebench::APIBridge::DataType data_type) const
{
    assert(dataset->getParameterCount() == m_op_input_count
           && dataset->getResultCount() == m_op_output_count);

    return BenchmarkLatency::validateResult(dataset, param_data_pack_indices, outputs, data_type);
}

} // namespace Latency
} // namespace GenericWL
} // namespace TestHarness
} // namespace hebench
