
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <cstring>
#include <sstream>

#include "hebench/api_bridge/api.h"
#include "modules/general/include/error.h"

#include "include/hebench_engine.h"

namespace hebench {
namespace TestHarness {

std::string Engine::getErrorDescription(hebench::APIBridge::ErrorCode err_code)
{
    std::vector<char> ch_retval;
    std::uint64_t n = hebench::APIBridge::getErrorDescription(err_code, nullptr, 0);
    if (n <= 0)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error retrieving error message from backend."));
    ch_retval.resize(n);
    if (hebench::APIBridge::getErrorDescription(err_code, ch_retval.data(), ch_retval.size()) <= 0)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error retrieving error message from backend."));
    return ch_retval.data();
}

std::string Engine::getLastErrorDescription() const
{
    std::vector<char> ch_retval;
    std::uint64_t n = hebench::APIBridge::getLastErrorDescription(m_handle, nullptr, 0);
    if (n <= 0)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error retrieving last error message from backend."));
    ch_retval.resize(n);
    if (hebench::APIBridge::getLastErrorDescription(m_handle, ch_retval.data(), ch_retval.size()) <= 0)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error retrieving last error message from backend."));
    return ch_retval.data();
}

void Engine::validateRetCode(hebench::APIBridge::ErrorCode err_code, bool last_error) const
{
    if (err_code != HEBENCH_ECODE_SUCCESS)
    {
        std::stringstream ss;
        ss << getErrorDescription(err_code);
        if (last_error)
        {
            std::string s = getLastErrorDescription();
            if (!s.empty())
                ss << std::endl
                   << s;
        } // end if
        throw hebench::Common::ErrorException(ss.str(), err_code);
    } // end if
}

Engine::Ptr Engine::create()
{
    Engine::Ptr retval = Engine::Ptr(new Engine());
    retval->init();
    return retval;
}

Engine::Engine()
{
    std::memset(&m_handle, 0, sizeof(m_handle));
}

void Engine::init()
{
    std::stringstream ss;

    // initialize backend engine
    hebench::APIBridge::ErrorCode err_code = hebench::APIBridge::initEngine(&m_handle);
    if (err_code != HEBENCH_ECODE_SUCCESS)
    {
        try
        {
            std::string s = getErrorDescription(err_code);
            ss            = std::stringstream(s);
            ss << s;
            s = getLastErrorDescription();
            if (!s.empty())
                ss << std::endl
                   << s;
        }
        catch (...)
        {
            ss = std::stringstream(IL_LOG_MSG_CLASS("Error initializing backend engine."));
        }
        throw hebench::Common::ErrorException(ss.str(), err_code);
    } // end if

    // register benchmarks from backend
    std::uint64_t count;
    validateRetCode(hebench::APIBridge::subscribeBenchmarksCount(m_handle, &count));
    m_h_bench_desc.resize(count);
    validateRetCode(hebench::APIBridge::subscribeBenchmarks(m_handle, m_h_bench_desc.data()));
}

Engine::~Engine()
{
    // destroy handles for benchmark descriptors
    for (const hebench::APIBridge::Handle &h : m_h_bench_desc)
        hebench::APIBridge::destroyHandle(h);
    // destroy handle for the engine
    hebench::APIBridge::destroyHandle(m_handle);
}

std::string Engine::getSchemeName(hebench::APIBridge::Scheme s) const
{
    std::vector<char> ch_retval;
    std::uint64_t n = hebench::APIBridge::getSchemeName(this->handle(), s, nullptr, 0);
    if (n <= 0)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error retrieving scheme name."));
    ch_retval.resize(n);
    if (hebench::APIBridge::getSchemeName(this->handle(), s,
                                          ch_retval.data(), ch_retval.size())
        <= 0)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error retrieving scheme name."));
    return ch_retval.data();
}

std::string Engine::getSecurityName(hebench::APIBridge::Scheme s,
                                    hebench::APIBridge::Security sec) const
{
    std::vector<char> ch_retval;
    std::uint64_t n = hebench::APIBridge::getSchemeSecurityName(this->handle(),
                                                                s,
                                                                sec,
                                                                nullptr, 0);
    if (n <= 0)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error retrieving security name."));
    ch_retval.resize(n);
    if (hebench::APIBridge::getSchemeSecurityName(this->handle(),
                                                  s,
                                                  sec,
                                                  ch_retval.data(), ch_retval.size())
        <= 0)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error retrieving security name."));
    return ch_retval.data();
}

std::string Engine::getExtraDescription(hebench::APIBridge::Handle h_bench_desc,
                                        const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const
{
    std::vector<char> ch_retval;
    const hebench::APIBridge::WorkloadParams hebench_w_params = { const_cast<hebench::APIBridge::WorkloadParam *>(w_params.data()), w_params.size() };
    const hebench::APIBridge::WorkloadParams *p_w_params      = w_params.empty() ? nullptr : &hebench_w_params;
    std::uint64_t n                                           = hebench::APIBridge::getBenchmarkDescriptionEx(this->handle(), h_bench_desc, p_w_params,
                                                                    nullptr, 0);
    if (n <= 0)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error retrieving extra description."));
    ch_retval.resize(n);
    n = hebench::APIBridge::getBenchmarkDescriptionEx(this->handle(), h_bench_desc, p_w_params,
                                                      ch_retval.data(), ch_retval.size());
    if (n <= 0)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error retrieving extra description."));
    return ch_retval.data();
}

IBenchmark::Ptr Engine::createBenchmark(IBenchmarkDescriptor::DescriptionToken::Ptr p_token, hebench::Utilities::TimingReportEx &out_report)
{
    if (!m_last_benchmark.expired())
        throw std::logic_error(IL_LOG_MSG_CLASS("A benchmark already exists. Cannot create more than one benchmark at a time."));

    IBenchmark::Ptr p_retval = BenchmarkFactory::createBenchmark(this->shared_from_this(), p_token, out_report);
    m_last_benchmark         = p_retval; // keep track of this benchmark
    return p_retval;
}

IBenchmarkDescriptor::DescriptionToken::Ptr Engine::describeBenchmark(std::size_t index,
                                                                      const BenchmarkDescription::Configuration &config) const
{
    if (index >= m_h_bench_desc.size())
        throw std::out_of_range(IL_LOG_MSG_CLASS("Total benchmarks: " + std::to_string(m_h_bench_desc.size()) + ". Invalid 'index' out of range: " + std::to_string(index) + "."));

    IBenchmarkDescriptor::DescriptionToken::Ptr retval;

    BenchmarkDescription::Backend backend_desc;
    backend_desc.m_backend_description.m_index  = index;
    backend_desc.m_backend_description.m_handle = m_h_bench_desc[index];
    validateRetCode(hebench::APIBridge::describeBenchmark(m_handle,
                                                          m_h_bench_desc[index],
                                                          &backend_desc.m_backend_description.m_descriptor,
                                                          nullptr));

    retval = BenchmarkFactory::matchBenchmarkDescriptor(*this, backend_desc, config);

    return retval;
}

std::vector<std::vector<hebench::APIBridge::WorkloadParam>>
Engine::getDefaultWorkloadParams(std::size_t index) const
{
    if (index >= m_h_bench_desc.size())
        throw std::out_of_range(IL_LOG_MSG_CLASS("Total benchmarks: " + std::to_string(m_h_bench_desc.size()) + ". Invalid 'index' out of range: " + std::to_string(index) + "."));

    std::uint64_t param_count, default_count;
    validateRetCode(hebench::APIBridge::getWorkloadParamsDetails(m_handle,
                                                                 m_h_bench_desc[index],
                                                                 &param_count,
                                                                 &default_count));
    std::vector<std::vector<hebench::APIBridge::WorkloadParam>> retval(default_count);
    if (!retval.empty())
    {
        std::vector<hebench::APIBridge::WorkloadParams> wps(retval.size());
        for (std::size_t i = 0; i < retval.size(); ++i)
        {
            retval[i].resize(param_count);
            wps[i].count  = retval[i].size();
            wps[i].params = retval[i].data();
        } // end for
        hebench::APIBridge::BenchmarkDescriptor tmp_bd;
        validateRetCode(hebench::APIBridge::describeBenchmark(m_handle,
                                                              m_h_bench_desc[index],
                                                              &tmp_bd,
                                                              wps.data()));
    } // end if

    return retval;
}

} // namespace TestHarness
} // namespace hebench
