
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <bitset>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "modules/logging/include/logging.h"
#include "modules/timer/include/timer.h"

#include "hebench/api_bridge/api.h"
#include "include/hebench_engine.h"
#include "include/hebench_ibenchmark.h"
#include "include/hebench_utilities_harness.h"

namespace hebench {
namespace TestHarness {

IBenchmarkDescriptor::DescriptionToken::Ptr IBenchmarkDescriptor::createToken(const BenchmarkDescription::Backend &backend_desc,
                                                                              const BenchmarkDescription::Configuration &config,
                                                                              const BenchmarkDescription::Description &text_desc) const
{
    return DescriptionToken::Ptr(new DescriptionToken(*const_cast<IBenchmarkDescriptor *>(this),
                                                      backend_desc,
                                                      config,
                                                      text_desc,
                                                      m_key_creation));
}

//-----------------------------------
// class PartialBenchmarkDescription
//-----------------------------------

PartialBenchmarkDescriptor::PartialBenchmarkDescriptor()
{
}

PartialBenchmarkDescriptor::~PartialBenchmarkDescriptor()
{
}

std::unordered_set<std::size_t> PartialBenchmarkDescriptor::getCipherParamPositions(std::uint32_t cipher_param_mask)
{
    std::unordered_set<std::size_t> retval;
    std::bitset<sizeof(decltype(cipher_param_mask)) * 8> cipher_param_bits(cipher_param_mask);
    for (std::size_t i = 0; i < cipher_param_bits.size(); ++i)
        if (cipher_param_bits.test(i))
            retval.insert(i);
    return retval;
}

std::string PartialBenchmarkDescriptor::getCategoryName(hebench::APIBridge::Category category)
{
    std::string retval;

    switch (category)
    {
    case hebench::APIBridge::Category::Latency:
        retval = "Latency";
        break;

    case hebench::APIBridge::Category::Offline:
        retval = "Offline";
        break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Unknown category: " + std::to_string(static_cast<int>(category)) + "."));
        break;
    } // end switch

    return retval;
}

std::string PartialBenchmarkDescriptor::getDataTypeName(hebench::APIBridge::DataType data_type)
{
    std::string retval;

    switch (data_type)
    {
    case hebench::APIBridge::DataType::Int32:
        retval = "Int32";
        break;

    case hebench::APIBridge::DataType::Int64:
        retval = "Int64";
        break;

    case hebench::APIBridge::DataType::Float32:
        retval = "Float32";
        break;

    case hebench::APIBridge::DataType::Float64:
        retval = "Float64";
        break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Unknown data type: " + std::to_string(static_cast<int>(data_type)) + "."));
        break;
    } // end switch

    return retval;
}

std::uint64_t PartialBenchmarkDescriptor::computeSampleSizes(std::uint64_t *sample_sizes,
                                                             std::size_t param_count,
                                                             const std::vector<std::uint64_t> &default_sample_sizes,
                                                             const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                                             std::uint64_t default_sample_size_fallback)
{
    if (bench_desc.category != hebench::APIBridge::Category::Offline)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid category. Only \"Offline\" category is supported."));
    if (default_sample_size_fallback < 1)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Default sample size fallback must be a positive value."));

    std::uint64_t result_batch_size = 1;
    for (std::size_t param_i = 0; param_i < param_count; ++param_i)
    {
        std::uint64_t default_sample_size =
            default_sample_sizes.size() > param_i && default_sample_sizes[param_i] > 0 ?
                default_sample_sizes[param_i] :
                default_sample_size_fallback;
        sample_sizes[param_i] = (bench_desc.cat_params.offline.data_count[param_i] == 0 ?
                                     default_sample_size :
                                     bench_desc.cat_params.offline.data_count[param_i]);
        result_batch_size *= sample_sizes[param_i];
    } // end for
    return result_batch_size;
}

IBenchmarkDescriptor::DescriptionToken::Ptr PartialBenchmarkDescriptor::matchDescriptor(const Engine &engine,
                                                                                        const BenchmarkDescription::Backend &backend_desc,
                                                                                        const BenchmarkDescription::Configuration &config) const
{
    IBenchmarkDescriptor::DescriptionToken::Ptr retval;

    BenchmarkDescription::Backend final_backend_desc = backend_desc;
    BenchmarkDescription::Configuration final_config = config;
    const auto &w_params                             = final_config.w_params;
    std::uint64_t w_params_count, other;
    engine.validateRetCode(hebench::APIBridge::getWorkloadParamsDetails(engine.handle(), final_backend_desc.handle, &w_params_count, &other));
    if (w_params_count != w_params.size())
    {
        std::stringstream ss;
        ss << "Invalid number of workload arguments. Expected " << w_params_count << ", but " << w_params.size() << " received.";
        throw std::runtime_error(IL_LOG_MSG_CLASS(ss.str()));
    } // end if

    if (matchBenchmarkDescriptor(final_backend_desc.descriptor, w_params))
    {
        // complete the benchmark specification
        BenchmarkDescription::Description text_description;
        describe(engine, final_backend_desc, final_config, text_description);
        retval = createToken(final_backend_desc, final_config, text_description);
    } // end if

    return retval;
}

void PartialBenchmarkDescriptor::describe(const Engine &engine,
                                          BenchmarkDescription::Backend &backend_desc,
                                          BenchmarkDescription::Configuration &config,
                                          BenchmarkDescription::Description &description) const
{
    hebench::APIBridge::Handle h_bench_desc                        = backend_desc.handle;
    const std::vector<hebench::APIBridge::WorkloadParam> &w_params = config.w_params;

    // obtain the values for the description that are specific to the workload
    WorkloadDescriptionOutput completed_description;
    std::memset(&completed_description.concrete_descriptor, 0, sizeof(completed_description.concrete_descriptor));
    completeWorkloadDescription(completed_description,
                                engine, backend_desc, config);

    const hebench::APIBridge::BenchmarkDescriptor &bench_desc = completed_description.concrete_descriptor;

    std::stringstream ss;
    std::filesystem::path ss_path;

    std::string &s_workload_name = completed_description.workload_name;
    std::string s_path_workload_name;
    std::string s_scheme_name   = engine.getSchemeName(bench_desc.scheme);
    std::string s_security_name = engine.getSecurityName(bench_desc.scheme, bench_desc.security);

    // generate path
    ss = std::stringstream();
    if (!s_workload_name.empty())
    {
        ss << s_workload_name << "_";
    }
    else
    {
        s_workload_name = std::to_string(static_cast<int>(bench_desc.workload));
    }
    ss << std::to_string(static_cast<int>(bench_desc.workload));
    s_path_workload_name = hebench::Utilities::convertToDirectoryName(ss.str());
    ss_path              = s_path_workload_name;
    ss                   = std::stringstream();
    ss << "wp";
    for (std::size_t i = 0; i < w_params.size(); ++i)
    {
        ss << "_";
        switch (w_params[i].data_type)
        {
        case hebench::APIBridge::WorkloadParamType::UInt64:
            ss << w_params[i].u_param;
            break;

        case hebench::APIBridge::WorkloadParamType::Float64:
            ss << w_params[i].f_param;
            break;

        default:
            ss << w_params[i].i_param;
            break;
        } // end switch
    } // end for
    ss_path /= hebench::Utilities::convertToDirectoryName(ss.str()); // workload params
    ss_path /= hebench::Utilities::convertToDirectoryName(PartialBenchmarkDescriptor::getCategoryName(bench_desc.category));
    ss_path /= hebench::Utilities::convertToDirectoryName(PartialBenchmarkDescriptor::getDataTypeName(bench_desc.data_type));

    ss = std::stringstream();
    ss << bench_desc.cat_params.min_test_time_ms << "ms";
    std::size_t max_non_zero = HEBENCH_MAX_CATEGORY_PARAMS;
    while (max_non_zero > 0 && bench_desc.cat_params.reserved[max_non_zero - 1] == 0)
        --max_non_zero;
    if (max_non_zero > 0)
    {
        for (std::size_t i = 0; i < max_non_zero; ++i)
            ss << "_" << bench_desc.cat_params.reserved[i];
    } // end if
    else
        ss << "_default";

    ss_path /= ss.str();
    // cipher/plain parameters
    ss = std::stringstream();
    std::unordered_set<std::size_t> cipher_param_pos =
        PartialBenchmarkDescriptor::getCipherParamPositions(bench_desc.cipher_param_mask);
    if (cipher_param_pos.empty())
        ss << "all_plain";
    else if (cipher_param_pos.size() >= sizeof(std::uint32_t) * 8)
        ss << "all_cipher";
    else
    {
        std::size_t max_elem = *std::max_element(cipher_param_pos.begin(), cipher_param_pos.end());
        for (std::size_t i = 0; i <= max_elem; ++i)
            ss << (cipher_param_pos.count(i) > 0 ? 'c' : 'p');
    } // end else
    std::string pt_ct_str = ss.str();
    ss_path /= pt_ct_str;
    ss_path /= hebench::Utilities::convertToDirectoryName(s_scheme_name);
    ss_path /= hebench::Utilities::convertToDirectoryName(s_security_name);
    ss_path /= std::to_string(bench_desc.other);
    // generate header
    ss = std::stringstream();
    ss << "Specifications," << std::endl
       << ", Encryption, " << std::endl
       << ", , Scheme, " << s_scheme_name << std::endl
       << ", , Security, " << s_security_name << std::endl
       << ", Extra, " << bench_desc.other << std::endl;
    std::string s_tmp = engine.getExtraDescription(h_bench_desc, w_params);
    if (!s_tmp.empty())
        ss << s_tmp;
    ss << std::endl
       << std::endl
       << ", Category, " << PartialBenchmarkDescriptor::getCategoryName(bench_desc.category) << std::endl
       << ", , Minimum test time requested (ms), " << bench_desc.cat_params.min_test_time_ms << std::endl;
    switch (bench_desc.category)
    {
    case hebench::APIBridge::Category::Latency:
        ss << ", , Warmup iterations, " << bench_desc.cat_params.latency.warmup_iterations_count << std::endl;
        break;

    case hebench::APIBridge::Category::Offline:
    {
        ss << ", , Parameter, Samples requested" << std::endl;

        bool all_params_zero = true;
        for (std::size_t i = 0; i < HEBENCH_MAX_OP_PARAMS; ++i)
        {
            if (bench_desc.cat_params.offline.data_count[i] != 0)
            {
                all_params_zero = false;
                ss << ", , " << i << ", " << bench_desc.cat_params.offline.data_count[i] << std::endl;
            } // end if
        } // end if
        if (all_params_zero)
            ss << ", , All, 0" << std::endl;
    }
    break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Unsupported benchmark category: " + std::to_string(bench_desc.category) + "."));
        break;
    } // end switch

    ss << std::endl
       << ", Workload, " << s_workload_name << std::endl
       << ", , Data type, " << PartialBenchmarkDescriptor::getDataTypeName(bench_desc.data_type) << std::endl
       << ", , Encrypted op parameters (index)";
    if (cipher_param_pos.empty())
        ss << ", None" << std::endl;
    else if (cipher_param_pos.size() >= sizeof(std::uint32_t) * 8)
        ss << ", All" << std::endl;
    else
    {
        // output the indices of the encrypted parameters
        std::vector<std::size_t> cipher_params(cipher_param_pos.begin(), cipher_param_pos.end());
        std::sort(cipher_params.begin(), cipher_params.end());
        for (auto param_index : cipher_params)
            ss << ", " << param_index;
        ss << std::endl;
    } // end else

    // append the extra header received from the workload
    ss << completed_description.workload_header;

    // return the generated description

    // make sure we have default sample sizes for every operation parameter
    Engine::completeBenchmarkDescriptor(backend_desc, completed_description.concrete_descriptor);
    backend_desc.operation_params_count = completed_description.operation_params_count;
    config.default_sample_sizes.resize(backend_desc.operation_params_count, 0);

    description.workload      = static_cast<std::int64_t>(bench_desc.workload);
    description.workload_name = completed_description.workload_base_name;
    description.data_type     = PartialBenchmarkDescriptor::getDataTypeName(bench_desc.data_type);
    description.category      = PartialBenchmarkDescriptor::getCategoryName(bench_desc.category);
    description.scheme        = engine.getSchemeName(bench_desc.scheme);
    description.security      = engine.getSecurityName(bench_desc.scheme, bench_desc.security);
    description.cipher_flags  = pt_ct_str;
    description.other         = std::to_string(completed_description.concrete_descriptor.other);
    description.header        = ss.str();
    description.path          = ss_path;
    if (config.b_single_path_report)
        // replace path separator by hyphens when requested
        std::replace(description.path.begin(), description.path.end(),
                     hebench::TestHarness::separator, hebench::TestHarness::hyphen);
}

//------------------------
// class PartialBenchmark
//------------------------

PartialBenchmark::PartialBenchmark(std::shared_ptr<Engine> p_engine,
                                   const IBenchmarkDescriptor::DescriptionToken &description_token) :
    m_current_event_id(0),
    m_b_constructed(false),
    m_b_initialized(false)
{
    if (!p_engine)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null argument 'p_engine'."));

    m_p_engine = p_engine; // this prevents engine from being destroyed while this object exists
    std::memset(&m_handle, 0, sizeof(m_handle));

    internalInit(description_token);

    m_b_constructed = true;
}

PartialBenchmark::~PartialBenchmark()
{
    // destroy benchmark handle
    hebench::APIBridge::destroyHandle(m_handle);
    // decouple from engine
    m_p_engine.reset(); // this object no longer prevents engine from being destroyed
}

void PartialBenchmark::internalInit(const IBenchmarkDescriptor::DescriptionToken &description_token)
{
    // cache description info about this benchmark
    m_backend_desc     = description_token.getBackendDescription();
    m_config           = description_token.getBenchmarkConfiguration();
    m_text_description = description_token.getDescription();
}

void PartialBenchmark::postInit()
{
    m_b_initialized = true;
}

void PartialBenchmark::initBackend(hebench::Utilities::TimingReportEx &out_report, const FriendPrivateKey &)
{
    hebench::Common::EventTimer timer;
    hebench::Common::TimingReportEvent::Ptr p_timing_event;
    std::stringstream ss;

    hebench::APIBridge::WorkloadParams params;
    params.count  = m_config.w_params.size();
    params.params = m_config.w_params.data();

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Creating backend benchmark...") << std::endl;
    timer.start();
    validateRetCode(hebench::APIBridge::createBenchmark(m_p_engine->handle(),
                                                        this->getBackendDescription().handle,
                                                        m_config.w_params.empty() ? nullptr : &params,
                                                        &m_handle));
    p_timing_event = timer.stop<DefaultTimeInterval>(getEventIDNext(), 1, nullptr);
    out_report.addEvent<DefaultTimeInterval>(p_timing_event, std::string("Creation"));
    hebench::Logging::GlobalLogger::log("OK");
    std::cout << IOS_MSG_OK << std::endl;

    std::cout << IOS_MSG_INFO << hebench::Logging::GlobalLogger::log("Initializing backend benchmark...") << std::endl;
    timer.start();
    validateRetCode(hebench::APIBridge::initBenchmark(m_handle, &this->getBackendDescription().descriptor));
    p_timing_event = timer.stop<DefaultTimeInterval>(getEventIDNext(), 1, nullptr);
    out_report.addEvent<DefaultTimeInterval>(p_timing_event, std::string("Initialization"));
    hebench::Logging::GlobalLogger::log("OK");
    std::cout << IOS_MSG_OK << std::endl;
}

void PartialBenchmark::checkInitializationState(const FriendPrivateKey &) const
{
    if (!m_b_constructed || !m_b_initialized)
        throw std::runtime_error(IL_LOG_MSG_CLASS("Initialization incomplete. All initialization stages must be called: init(), initBackend(), postInit()."));
}

void PartialBenchmark::validateRetCode(hebench::APIBridge::ErrorCode err_code, bool last_error) const
{
    m_p_engine->validateRetCode(err_code, last_error);
}

} // namespace TestHarness
} // namespace hebench
