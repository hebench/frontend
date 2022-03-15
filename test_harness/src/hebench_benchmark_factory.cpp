
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <bitset>
#include <cassert>
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
                                                  IBenchmarkDescriptor::DescriptionToken::Ptr p_token,
                                                  hebench::Utilities::TimingReportEx &out_report)
{
    if (!p_engine)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null pointer \"p_engine\"."));
    if (!p_token)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null pointer \"p_token\"."));
    if (!p_token->getDescriptor())
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid benchmark token \"p_token\" with null descriptor."));

    std::vector<std::shared_ptr<IBenchmarkDescriptor>> &registered_benchmarks = getRegisteredBenchmarks();
    // make sure that the token matches to a descriptor that is registered
    IBenchmarkDescriptor *p_tmp = p_token->getDescriptor();
    auto found_it               = std::find_if(registered_benchmarks.begin(), registered_benchmarks.end(),
                                 [p_tmp](std::shared_ptr<IBenchmarkDescriptor> p) -> bool { return p_tmp == p.get(); });
    if (found_it == registered_benchmarks.end() || !*found_it)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Benchmark token \"p_token\" descriptor is invalid: not found in registered benchmark descriptors."));
    std::shared_ptr<IBenchmarkDescriptor> p_bd = *found_it; // this ensures that we are keeping the smart pointer around until after destruction.
    PartialBenchmark *p_retval                 = p_bd->createBenchmark(p_engine, *p_token);
    if (!p_retval)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error creating benchmark."));
    try
    {
        // perform initialization of the benchmark
        const PartialBenchmark::FriendPrivateKey key;
        p_retval->init();
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
                           [p_bd](IBenchmark *p) { if (p) p_bd->destroyBenchmark(reinterpret_cast<PartialBenchmark *>(p)); });
}

std::vector<std::shared_ptr<IBenchmarkDescriptor>> &BenchmarkFactory::getRegisteredBenchmarks()
{
    // ensures that the static member always exists during static initialization
    static std::vector<std::shared_ptr<IBenchmarkDescriptor>> registered_benchmarks;
    return registered_benchmarks;
}

IBenchmarkDescriptor::DescriptionToken::Ptr BenchmarkFactory::matchBenchmarkDescriptor(const Engine &engine,
                                                                                       const BenchmarkDescription::Backend &backend_desc,
                                                                                       const BenchmarkDescription::Configuration &config)
{
    IBenchmarkDescriptor::DescriptionToken::Ptr retval;

    std::vector<std::shared_ptr<IBenchmarkDescriptor>> &registered_benchmarks =
        getRegisteredBenchmarks();
    for (std::size_t i = 0; !retval && i < registered_benchmarks.size(); ++i)
        retval = registered_benchmarks[i]->matchBenchmarkDescriptor(engine, backend_desc, config);

    return retval;
}

bool BenchmarkFactory::registerSupportedBenchmark(std::shared_ptr<IBenchmarkDescriptor> p_desc_obj)
{
    bool retval = false;
    try
    {
        if (p_desc_obj)
        {
            getRegisteredBenchmarks().push_back(p_desc_obj);
            retval = true;
        } // end if
    }
    catch (...)
    {
        retval = false;
    }
    return retval;
}

} // namespace TestHarness
} // namespace hebench
