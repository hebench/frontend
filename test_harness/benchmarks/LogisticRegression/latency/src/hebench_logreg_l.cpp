
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

#include "../include/hebench_logreg_l.h"

namespace hebench {
namespace TestHarness {
namespace LogisticRegression {
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
    assert(OpParameterCount == 3);
    assert(DefaultBatchSize == 1);

    BenchmarkDescriptorCategory::completeWorkloadDescription(output, engine, backend_desc, config);

    assert(OpParameterCount == output.operation_params_count);

    // finish benchmark header description

    std::stringstream ss;
    std::uint64_t batch_sizes[OpParameterCount] = { 1, 1, DefaultBatchSize }; // W, b, X
    std::uint64_t vector_size                   = fetchVectorSize(config.w_params);

    std::uint64_t result_batch_size = 1;
    for (std::size_t param_i = 0; param_i < OpParameterCount; ++param_i)
        result_batch_size *= batch_sizes[param_i];

    // complete header with workload specifics
    ss << ", , P(X = X') = sigmoid";
    switch (backend_desc.descriptor.workload)
    {
    case hebench::APIBridge::Workload::LogisticRegression_PolyD3:
        ss << "_pd3";
        break;
    case hebench::APIBridge::Workload::LogisticRegression_PolyD5:
        ss << "_pd5";
        break;
    case hebench::APIBridge::Workload::LogisticRegression_PolyD7:
        ss << "_pd7";
        break;
    default:
        // standard sigmoid
        break;
    } // end switch
    ss << "(W . X + b)" << std::endl
       << ", , , Elements, Batch size" << std::endl;
    ss << ", , W, " << vector_size << ", " << batch_sizes[DataLoader::Index_W] << std::endl;
    ss << ", , b, 1, " << batch_sizes[DataLoader::Index_b] << std::endl;
    ss << ", , X, " << vector_size << ", " << batch_sizes[DataLoader::Index_X] << std::endl;
    ss << ", , P(X), 1, " << result_batch_size << std::endl;

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
    std::uint64_t vector_size;
    std::uint64_t batch_sizes[BenchmarkDescriptor::OpParameterCount] = { 1, 1, BenchmarkDescriptor::DefaultBatchSize }; // W, b, X
    std::stringstream ss;

    vector_size = BenchmarkDescriptor::fetchVectorSize(this->getBenchmarkConfiguration().w_params);

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Preparing workload.") << std::endl;

    DataLoader::PolynomialDegree pd;
    switch (this->getBackendDescription().descriptor.workload)
    {
    case hebench::APIBridge::Workload::LogisticRegression_PolyD3:
        pd = DataLoader::PolynomialDegree::PD3;
        break;
    case hebench::APIBridge::Workload::LogisticRegression_PolyD5:
        pd = DataLoader::PolynomialDegree::PD5;
        break;
    case hebench::APIBridge::Workload::LogisticRegression_PolyD7:
        pd = DataLoader::PolynomialDegree::PD7;
        break;
    default:
        pd = DataLoader::PolynomialDegree::None;
        break;
    } // end switch

    timer.start();
    if (this->getBenchmarkConfiguration().dataset_filename.empty())
    {
        // generates random values for input and generates (computes) ground truth
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Generating data...") << std::endl;
        m_data = DataLoader::create(pd,
                                    vector_size,
                                    batch_sizes[DataLoader::Index_X],
                                    this->getBackendDescription().descriptor.data_type);
    } // end if
    else
    {
        std::stringstream ss;
        ss << "Loading data from external dataset: " << std::endl
           << "\"" << this->getBenchmarkConfiguration().dataset_filename << "\"";
        std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log(ss.str()) << std::endl;
        // load values for input and ground truth from file
        m_data = DataLoader::create(pd,
                                    vector_size,
                                    batch_sizes[DataLoader::Index_X],
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
} // namespace LogisticRegression
} // namespace TestHarness
} // namespace hebench
