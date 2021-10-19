
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

#include "../include/hebench_matmult_o.h"

namespace hebench {
namespace TestHarness {
namespace MatrixMultiply {
namespace Offline {

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

    // return true if benchmark is supported
    if (bench_desc.category == hebench::APIBridge::Category::Offline)
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
    assert(DefaultBatchSize == 100);

    std::stringstream ss;
    std::uint64_t default_batch_size = pre_token->getBenchmarkConfiguration(this).default_sample_size > 0 ?
                                           pre_token->getBenchmarkConfiguration(this).default_sample_size :
                                           BenchmarkDescription::DefaultBatchSize;
    std::uint64_t batch_sizes[OpParameterCount];
    std::array<std::pair<std::uint64_t, std::uint64_t>, OpParameterCount> mat_dims =
        fetchMatrixSizes(pre_token->getWorkloadParams(this));

    ss = std::stringstream();
    ss << pre_token->description.header;

    std::uint64_t result_batch_size = computeSampleSizes(batch_sizes,
                                                         OpParameterCount,
                                                         default_batch_size,
                                                         pre_token->getDescriptor(this));

    // complete header with workload specifics
    ss << ", , M = M0 x M1" << std::endl
       << ", , , Rows, Columns, Batch size" << std::endl;
    for (std::size_t i = 0; i < OpParameterCount; ++i)
    {
        ss << ", , M" << i << ", " << mat_dims[i].first << ", " << mat_dims[i].second << ", " << batch_sizes[i] << std::endl;
    } // end for
    ss << ", , M, " << mat_dims[0].first << ", " << mat_dims[1].second << ", " << result_batch_size << std::endl;

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
    BenchmarkOffline(p_engine, description_token)
{
}

void Benchmark::init(const IBenchmarkDescription::Description &description)
{
    (void)description;
    hebench::Common::EventTimer timer;
    hebench::Common::TimingReportEvent::Ptr p_timing_event;
    std::uint64_t default_batch_size = m_benchmark_configuration.default_sample_size > 0 ?
                                           m_benchmark_configuration.default_sample_size :
                                           BenchmarkDescription::DefaultBatchSize;
    std::uint64_t batch_sizes[BenchmarkDescription::OpParameterCount];
    std::stringstream ss;

    std::array<std::pair<std::uint64_t, std::uint64_t>, BenchmarkDescription::OpParameterCount> mat_dims =
        BenchmarkDescription::fetchMatrixSizes(m_params);

    BenchmarkDescription::computeSampleSizes(batch_sizes,
                                             BenchmarkDescription::OpParameterCount,
                                             default_batch_size,
                                             m_descriptor);

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Generating workload...") << std::endl;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Loading workload data...") << std::endl;

    timer.start();
    // generates random matrices for input and generates (computes) ground truth
    m_data         = DataGenerator::create(mat_dims[0].first, mat_dims[0].second, // M0
                                           mat_dims[1].second, // M1
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

    return BenchmarkOffline::validateResult(dataset, param_data_pack_indices, outputs, data_type);
}

} // namespace Offline
} // namespace MatrixMultiply
} // namespace TestHarness
} // namespace hebench
