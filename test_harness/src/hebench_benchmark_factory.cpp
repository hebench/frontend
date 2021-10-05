
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <bitset>
#include <filesystem>
#include <sstream>

#include "hebench/api_bridge/api.h"

#include "include/hebench_benchmark_factory.h"

namespace hebench {
namespace TestHarness {

//------------------------
// class BenchmarkFactory
//------------------------

IBenchmark::Ptr BenchmarkFactory::createBenchmark(std::shared_ptr<Engine> p_engine,
                                                  BenchmarkToken::Ptr p_token,
                                                  hebench::Utilities::TimingReportEx &out_report)
{
    if (!p_engine)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null pointer \"p_engine\"."));
    if (!p_token)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null pointer \"p_token\"."));
    if (!p_token->m_p_bmd || !p_token->m_p_bdt)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid benchmark token \"p_token\"."));

    std::shared_ptr<IBenchmarkDescription> p_bd = p_token->m_p_bmd;
    PartialBenchmark *p_retval =
        p_bd->createBenchmark(p_engine, p_token->m_p_bdt);
    if (!p_retval)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error creating benchmark."));
    try
    {
        // perform initialization of the benchmark
        const PartialBenchmark::FriendPrivateKey key;
        p_retval->init(p_token->description);
        p_retval->initBackend(out_report, key);
        p_retval->postInit();
        p_retval->checkInitializationState(key);
    }
    catch (...)
    {
        try
        {
            p_bd->destroyBenchmark(p_retval);
        }
        catch (...)
        {
            // ignore any exceptions on destruction
        }
        throw;
    }
    return IBenchmark::Ptr(p_retval,
                           [p_bd](IBenchmark *p) { p_bd->destroyBenchmark(reinterpret_cast<PartialBenchmark *>(p)); });
}

std::vector<std::shared_ptr<IBenchmarkDescription>> &BenchmarkFactory::getRegisteredBenchmarks()
{
    // ensures that the static member always exists during static initialization
    static std::vector<std::shared_ptr<IBenchmarkDescription>> registered_benchmarks;
    return registered_benchmarks;
}

BenchmarkFactory::BenchmarkToken::Ptr BenchmarkFactory::matchBenchmarkDescriptor(const Engine &engine,
                                                                                 const IBenchmarkDescription::BenchmarkConfig &bench_config,
                                                                                 const hebench::APIBridge::Handle &h_desc,
                                                                                 const std::vector<hebench::APIBridge::WorkloadParam> &w_params)
{
    BenchmarkFactory::BenchmarkToken::Ptr retval;

    std::vector<std::shared_ptr<IBenchmarkDescription>> &registered_benchmarks =
        getRegisteredBenchmarks();
    for (std::size_t i = 0; !retval && i < registered_benchmarks.size(); ++i)
    {
        IBenchmarkDescription::DescriptionToken::Ptr p_matched_benchmark =
            registered_benchmarks[i]->matchBenchmarkDescriptor(engine, bench_config, h_desc, w_params);
        if (p_matched_benchmark)
            retval = createBenchmarkToken(registered_benchmarks[i], p_matched_benchmark);
    } // end for

    return retval;
}

BenchmarkFactory::BenchmarkToken::Ptr BenchmarkFactory::createBenchmarkToken(std::shared_ptr<IBenchmarkDescription> p_bmd,
                                                                             IBenchmarkDescription::DescriptionToken::Ptr p_bdt)
{
    BenchmarkFactory::BenchmarkToken::Ptr retval;

    if (p_bmd && p_bdt)
        retval = BenchmarkFactory::BenchmarkToken::Ptr(new BenchmarkFactory::BenchmarkToken(p_bmd, p_bdt));

    return retval;
}

bool BenchmarkFactory::registerSupportedBenchmark(std::shared_ptr<IBenchmarkDescription> p_desc_obj)
{
    bool retval = true;
    try
    {
        getRegisteredBenchmarks().push_back(p_desc_obj);
    }
    catch (...)
    {
        retval = false;
    }
    return retval;
}

} // namespace TestHarness
} // namespace hebench
