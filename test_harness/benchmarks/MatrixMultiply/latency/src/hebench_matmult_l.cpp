
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

#include "../include/hebench_matmult_l.h"

namespace hebench {
namespace TestHarness {
namespace MatrixMultiply {
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
    assert(OpParameterCount == 2);
    assert(DefaultBatchSize == 1);

    BenchmarkDescriptorCategory::completeWorkloadDescription(output, engine, backend_desc, config);

    assert(OpParameterCount == output.operation_params_count);

    // finish benchmark header description

    std::stringstream ss;
    std::uint64_t batch_sizes[OpParameterCount];
    std::array<std::pair<std::uint64_t, std::uint64_t>, OpParameterCount> mat_dims =
        fetchMatrixSizes(config.w_params);

    ss = std::stringstream();

    std::uint64_t result_batch_size = 1;
    for (std::size_t param_i = 0; param_i < OpParameterCount; ++param_i)
    {
        batch_sizes[param_i] = DefaultBatchSize;
        result_batch_size *= batch_sizes[param_i];
    } // end for
    // complete header with workload specifics
    ss << ", , M = M0 x M1" << std::endl
       << ", , , Rows, Columns, Batch size" << std::endl;
    for (std::size_t i = 0; i < OpParameterCount; ++i)
    {
        ss << ", , M" << i << ", " << mat_dims[i].first << ", " << mat_dims[i].second << ", " << batch_sizes[i] << std::endl;
    } // end for
    ss << ", , M, " << mat_dims[0].first << ", " << mat_dims[1].second << ", " << result_batch_size << std::endl;

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
    BenchmarkLatency(p_engine, description_token)
{
}

void Benchmark::init()
{
    hebench::Common::EventTimer timer;
    hebench::Common::TimingReportEvent::Ptr p_timing_event;
    std::uint64_t batch_sizes[BenchmarkDescriptor::OpParameterCount];
    std::stringstream ss;

    std::array<std::pair<std::uint64_t, std::uint64_t>, BenchmarkDescriptor::OpParameterCount> mat_dims =
        BenchmarkDescriptor::fetchMatrixSizes(this->getBenchmarkConfiguration().w_params);
    for (std::size_t param_i = 0; param_i < BenchmarkDescriptor::OpParameterCount; ++param_i)
        batch_sizes[param_i] = BenchmarkDescriptor::DefaultBatchSize;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Generating workload...") << std::endl;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Loading workload data...") << std::endl;

    timer.start();
    if (this->getBenchmarkConfiguration().dataset_filename.empty())
    {
        // generates random matrices for input and generates (computes) ground truth
        m_data = DataLoader::create(mat_dims[0].first, mat_dims[0].second, // M0
                                    mat_dims[1].second, // M1
                                    batch_sizes[0], batch_sizes[1],
                                    this->getBackendDescription().descriptor.data_type);
    } // end if
    else
    {
        // load matrices for input and ground truth from file
        m_data = DataLoader::create(mat_dims[0].first, mat_dims[0].second, // M0
                                    mat_dims[1].second, // M1
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

    return BenchmarkLatency::validateResult(dataset, param_data_pack_indices, outputs, data_type);
}

} // namespace Latency
} // namespace MatrixMultiply
} // namespace TestHarness
} // namespace hebench
