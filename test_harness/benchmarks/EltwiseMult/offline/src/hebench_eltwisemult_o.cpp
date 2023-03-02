
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

#include "../include/hebench_eltwisemult_o.h"

namespace hebench {
namespace TestHarness {
namespace EltwiseMult {
namespace Offline {

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
        && (bench_desc.category == hebench::APIBridge::Category::Offline);

    return retval;
}

void BenchmarkDescriptor::completeWorkloadDescription(WorkloadDescriptionOutput &output,
                                                      const Engine &engine,
                                                      const BenchmarkDescription::Backend &backend_desc,
                                                      const BenchmarkDescription::Configuration &config) const
{
    // finish describing workload
    assert(OpParameterCount == 2);
    assert(DefaultBatchSize == 5);

    BenchmarkDescriptorCategory::completeWorkloadDescription(output, engine, backend_desc, config);

    assert(OpParameterCount == output.operation_params_count);

    std::stringstream ss;
    std::uint64_t *batch_sizes = output.concrete_descriptor.cat_params.offline.data_count;
    std::uint64_t vector_size  = fetchVectorSize(config.w_params);

    std::uint64_t sample_size_fallback =
        config.fallback_default_sample_size > 0 ?
            config.fallback_default_sample_size :
            DefaultBatchSize;
    std::uint64_t result_batch_size =
        PartialBenchmarkDescriptor::computeSampleSizes(batch_sizes,
                                                       OpParameterCount,
                                                       config.default_sample_sizes,
                                                       backend_desc.descriptor,
                                                       sample_size_fallback,
                                                       PartialBenchmarkDescriptor::getForceConfigValues());
    // complete header with workload specifics
    ss << ", , C = V0 * V1" << std::endl
       << ", , , Elements, Batch size" << std::endl;
    for (std::size_t i = 0; i < OpParameterCount; ++i)
    {
        ss << ", , V" << i << ", " << vector_size << ", " << batch_sizes[i] << std::endl;
    } // end for
    ss << ", , C, " << vector_size << ", " << result_batch_size << std::endl;

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
    BenchmarkOffline(p_engine, description_token)
{
}

void Benchmark::init()
{
    hebench::Common::EventTimer timer;
    hebench::Common::TimingReportEvent::Ptr p_timing_event;
    std::uint64_t batch_sizes[BenchmarkDescriptor::OpParameterCount];
    std::stringstream ss;

    std::uint64_t vector_size = BenchmarkDescriptor::fetchVectorSize(this->getBenchmarkConfiguration().w_params);

    std::uint64_t sample_size_fallback =
        this->getBenchmarkConfiguration().fallback_default_sample_size > 0 ?
            this->getBenchmarkConfiguration().fallback_default_sample_size :
            BenchmarkDescriptor::DefaultBatchSize;
    BenchmarkDescriptor::computeSampleSizes(batch_sizes,
                                            BenchmarkDescriptor::OpParameterCount,
                                            this->getBenchmarkConfiguration().default_sample_sizes,
                                            this->getBackendDescription().descriptor,
                                            sample_size_fallback,
                                            BenchmarkDescriptor::getForceConfigValues());

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Generating workload...") << std::endl;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Loading workload data...") << std::endl;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Preparing workload.") << std::endl;

    timer.start();
    if (this->getBenchmarkConfiguration().dataset_filename.empty())
    {
        // generates random vectors for input and generates (computes) ground truth
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Generating data...") << std::endl;
        m_data = DataLoader::create(vector_size,
                                    batch_sizes[0], batch_sizes[1],
                                    this->getBackendDescription().descriptor.data_type);
    } // end if
    else
    {
        std::stringstream ss;
        ss << "Loading data from external dataset: " << std::endl
           << "\"" << this->getBenchmarkConfiguration().dataset_filename << "\"";
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
        // load vectors for input and ground truth from file
        m_data = DataLoader::create(vector_size,
                                    batch_sizes[0], batch_sizes[1],
                                    this->getBackendDescription().descriptor.data_type,
                                    this->getBenchmarkConfiguration().dataset_filename);
    } // end else
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
    assert(dataset->getParameterCount() == BenchmarkDescriptorCategory::OpParameterCount
           && dataset->getResultCount() == BenchmarkDescriptorCategory::OpResultCount);

    return BenchmarkOffline::validateResult(dataset, param_data_pack_indices, outputs, data_type);
}

} // namespace Offline
} // namespace EltwiseMult
} // namespace TestHarness
} // namespace hebench
