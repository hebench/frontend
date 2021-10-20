
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

#include "../include/hebench_logreg_o.h"

namespace hebench {
namespace TestHarness {
namespace LogisticRegression {
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

    // return name if benchmark is supported
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
    assert(OpParameterCount == 3);
    assert(DefaultBatchSize == 100);

    std::uint64_t vector_size;
    std::uint64_t default_batch_size = pre_token->getBenchmarkConfiguration(this).default_sample_size > 0 ?
                                           pre_token->getBenchmarkConfiguration(this).default_sample_size :
                                           BenchmarkDescription::DefaultBatchSize;
    std::uint64_t batch_sizes[OpParameterCount];
    std::stringstream ss;
    hebench::APIBridge::BenchmarkDescriptor bench_desc = pre_token->getDescriptor(this);

    if (bench_desc.cat_params.offline.data_count[DataGenerator::Index_W] == 0)
        bench_desc.cat_params.offline.data_count[DataGenerator::Index_W] = 1;
    if (bench_desc.cat_params.offline.data_count[DataGenerator::Index_b] == 0)
        bench_desc.cat_params.offline.data_count[DataGenerator::Index_b] = 1;

    vector_size = fetchVectorSize(pre_token->getWorkloadParams(this));

    std::uint64_t result_batch_size = computeSampleSizes(batch_sizes,
                                                         OpParameterCount,
                                                         default_batch_size,
                                                         bench_desc);
    if (batch_sizes[DataGenerator::Index_W] != 1)
    {
        ss = std::stringstream();
        ss << "Batch size for feature vector 'W' must be 1, but " << batch_sizes[DataGenerator::Index_W] << " received.";
        throw std::invalid_argument(ss.str());
    } // end if
    if (batch_sizes[DataGenerator::Index_b] != 1)
    {
        ss = std::stringstream();
        ss << "Batch size for bias 'b' must be 1, but " << batch_sizes[DataGenerator::Index_b] << " received.";
        throw std::invalid_argument(ss.str());
    } // end if

    ss = std::stringstream();
    ss << pre_token->description.header;

    // complete header with workload specifics
    ss << ", , P(X = X') = sigmoid";
    switch (bench_desc.workload)
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
    ss << ", , W, " << vector_size << ", " << batch_sizes[DataGenerator::Index_W] << std::endl;
    ss << ", , b, 1, " << batch_sizes[DataGenerator::Index_b] << std::endl;
    ss << ", , X, " << vector_size << ", " << batch_sizes[DataGenerator::Index_X] << std::endl;
    ss << ", , P(X), 1, " << result_batch_size << std::endl;

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
    std::uint64_t vector_size;
    std::uint64_t default_batch_size = m_benchmark_configuration.default_sample_size > 0 ?
                                           m_benchmark_configuration.default_sample_size :
                                           BenchmarkDescription::DefaultBatchSize;
    std::uint64_t batch_sizes[BenchmarkDescription::OpParameterCount];
    std::stringstream ss;

    hebench::APIBridge::BenchmarkDescriptor bench_desc = m_descriptor;
    if (bench_desc.cat_params.offline.data_count[DataGenerator::Index_W] == 0)
        bench_desc.cat_params.offline.data_count[DataGenerator::Index_W] = 1;
    if (bench_desc.cat_params.offline.data_count[DataGenerator::Index_b] == 0)
        bench_desc.cat_params.offline.data_count[DataGenerator::Index_b] = 1;

    vector_size = BenchmarkDescription::fetchVectorSize(m_params);
    BenchmarkDescription::computeSampleSizes(batch_sizes,
                                             BenchmarkDescription::OpParameterCount,
                                             default_batch_size,
                                             bench_desc);

    assert(batch_sizes[DataGenerator::Index_W] == 1 && batch_sizes[DataGenerator::Index_b] == 1);

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Generating workload...") << std::endl;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Loading workload data...") << std::endl;

    DataGenerator::PolynomialDegree pd;
    switch (m_descriptor.workload)
    {
    case hebench::APIBridge::Workload::LogisticRegression_PolyD3:
        pd = DataGenerator::PolynomialDegree::PD3;
        break;
    case hebench::APIBridge::Workload::LogisticRegression_PolyD5:
        pd = DataGenerator::PolynomialDegree::PD5;
        break;
    case hebench::APIBridge::Workload::LogisticRegression_PolyD7:
        pd = DataGenerator::PolynomialDegree::PD7;
        break;
    default:
        pd = DataGenerator::PolynomialDegree::None;
        break;
    } // end switch

    timer.start();
    m_data         = DataGenerator::create(pd,
                                   vector_size,
                                   batch_sizes[DataGenerator::Index_X],
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
} // namespace LogisticRegression
} // namespace TestHarness
} // namespace hebench
