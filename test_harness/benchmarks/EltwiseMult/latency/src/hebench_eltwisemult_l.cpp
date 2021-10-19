
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

#include "modules/timer/include/timer.h"

#include "hebench/api_bridge/api.h"
#include "include/hebench_engine.h"
#include "include/hebench_math_utils.h"

#include "../include/hebench_eltwisemult_l.h"

namespace hebench {
namespace TestHarness {
namespace EltwiseMult {
namespace Latency {

//----------------------------
// class BenchmarkDescription
//----------------------------

bool BenchmarkDescription::m_b_registered = // register the benchmark with the factory
    hebench::TestHarness::BenchmarkFactory::registerSupportedBenchmark(std::make_shared<BenchmarkDescription>());

std::string BenchmarkDescription::matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                                           const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const
{
    assert(m_b_registered);

    std::string retval;

    // return name if benchmark is supported
    if (bench_desc.category == hebench::APIBridge::Category::Latency)
    {
        retval = BenchmarkDescriptionCategory::matchBenchmarkDescriptor(bench_desc, w_params);
    } // end if

    return retval;
}

void BenchmarkDescription::completeDescription(const Engine &engine,
                                               DescriptionToken::Ptr pre_token) const
{
    (void)engine;

    // finish describing workload
    assert(OpParameterCount == 2);
    assert(DefaultBatchSize == 1);

    std::uint64_t vector_size;
    std::uint64_t batch_sizes[OpParameterCount];
    std::stringstream ss;

    vector_size = fetchVectorSize(pre_token->getWorkloadParams(this));

    ss = std::stringstream();
    ss << pre_token->description.header;

    std::uint64_t result_batch_size = 1;
    for (std::size_t param_i = 0; param_i < OpParameterCount; ++param_i)
    {
        batch_sizes[param_i] = DefaultBatchSize;
        result_batch_size *= batch_sizes[param_i];
    } // end for
    // complete header with workload specifics
    ss << ", , C[i] = V0[i] * V1[i]" << std::endl
       << ", , , Elements, Batch size" << std::endl;
    for (std::size_t i = 0; i < OpParameterCount; ++i)
    {
        ss << ", , V" << i << ", " << vector_size << ", " << batch_sizes[i] << std::endl;
    } // end for
    ss << ", , C, " << vector_size << ", " << result_batch_size << std::endl;

    pre_token->description.header = ss.str();
}

hebench::TestHarness::PartialBenchmark *BenchmarkDescription::createBenchmark(std::shared_ptr<Engine> p_engine,
                                                                              DescriptionToken::Ptr p_description_token)
{
    assert(m_b_registered);
    Benchmark *retval = nullptr;

    try
    {
        if (!p_description_token)
            throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null argument 'p_description_token'."));
        retval = new Benchmark(p_engine, *p_description_token);
    }
    catch (...)
    {
        if (retval)
            delete retval;
        throw;
    }

    return retval;
}

void BenchmarkDescription::destroyBenchmark(PartialBenchmark *p_bench)
{
    assert(m_b_registered);
    if (p_bench)
        delete p_bench;
}

//-----------------
// class Benchmark
//-----------------

Benchmark::Benchmark(std::shared_ptr<Engine> p_engine,
                     const IBenchmarkDescription::DescriptionToken &description_token) :
    BenchmarkLatency(p_engine, description_token)
{
}

void Benchmark::init(const IBenchmarkDescription::Description &description)
{
    (void)description;
    hebench::Common::EventTimer timer;
    hebench::Common::TimingReportEvent::Ptr p_timing_event;
    std::uint64_t vector_size;
    std::uint64_t batch_sizes[BenchmarkDescription::OpParameterCount];
    std::stringstream ss;

    vector_size = BenchmarkDescription::fetchVectorSize(m_params);
    for (std::size_t param_i = 0; param_i < BenchmarkDescription::OpParameterCount; ++param_i)
        batch_sizes[param_i] = BenchmarkDescription::DefaultBatchSize;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Generating workload...") << std::endl;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Loading workload data...") << std::endl;

    timer.start();
    m_data         = DataGenerator::create(vector_size,
                                           batch_sizes[0], batch_sizes[1],
                                           m_descriptor.data_type);
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
    assert(dataset->getParameterCount() == BenchmarkDescriptionCategory::OpParameterCount
           && dataset->getResultCount() == BenchmarkDescriptionCategory::OpResultCount);

    return BenchmarkLatency::validateResult(dataset, param_data_pack_indices, outputs, data_type);
}

} // namespace Latency
} // namespace EltwiseMult
} // namespace TestHarness
} // namespace hebench
