
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <cassert>
#include <cctype>
#include <filesystem>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <yaml-cpp/yaml.h>

#include "include/hebench_benchmark_description.h"
#include "include/hebench_benchmark_factory.h"
#include "include/hebench_config.h"
#include "include/hebench_engine.h"
#include "include/hebench_ibenchmark.h"
#include "include/hebench_math_utils.h"
#include "include/hebench_utilities.h"

namespace hebench {
namespace Utilities {

class ConfigExporterImpl
{
public:
    /**
     * @brief exportBenchmarkDescription2YAML
     * @param out
     * @param bench_desc Fully described BenchmarkDescription object to export.
     */
    static void exportBenchmarkRequest2YAML(YAML::Emitter &out,
                                            const BenchmarkRequest &bench_req,
                                            const hebench::TestHarness::BenchmarkDescription::Description &description);

private:
    static YAML::Node createWorkloadParamYAMLNode(const hebench::APIBridge::WorkloadParam &w_param);

    template <typename T>
    static YAML::Node createWorkloadParamValueYAMLNode(const T &value)
    {
        YAML::Node retval;
        retval["from"] = value;
        retval["to"]   = value;
        retval["step"] = static_cast<T>(0); // step is always 0 for exports

        return retval;
    }
};

class ConfigImporterImpl
{
public:
    static std::vector<BenchmarkRequest> importYAML2BenchmarkRequest(std::size_t benchmark_index,
                                                                     const YAML::Node &yaml_bench,
                                                                     const hebench::TestHarness::Engine &engine,
                                                                     std::uint64_t fallback_min_test_time,
                                                                     std::uint64_t fallback_sample_size);

private:
    template <typename T>
    static std::size_t computeComponentSize(const YAML::Node &node_from,
                                            const YAML::Node &node_to,
                                            const YAML::Node &node_step)
    {
        T param_step = node_step.as<T>();
        if (param_step == static_cast<T>(0))
            param_step = std::numeric_limits<T>::max();
        T tmp_to   = node_to.as<T>();
        T tmp_from = node_from.as<T>();
        if (tmp_to < tmp_from)
            tmp_to = tmp_from;
        return static_cast<std::size_t>((tmp_to - tmp_from) / param_step) + 1;
    }

    template <typename T>
    static T computeParamValue(std::size_t count,
                               const YAML::Node &node_from,
                               const YAML::Node &node_step)
    {
        return static_cast<T>(count * node_step.as<T>() + node_from.as<T>());
    }
};

//--------------------------
// class ConfigImporterImpl
//--------------------------

std::vector<BenchmarkRequest> ConfigImporterImpl::importYAML2BenchmarkRequest(std::size_t benchmark_index,
                                                                              const YAML::Node &yaml_bench,
                                                                              const TestHarness::Engine &engine,
                                                                              std::uint64_t fallback_min_test_time,
                                                                              std::uint64_t fallback_sample_size)
{
    assert(yaml_bench["ID"].as<decltype(benchmark_index)>() == benchmark_index);

    std::vector<BenchmarkRequest> retval;

    std::uint64_t default_min_test_time_ms = 0;
    std::vector<std::uint64_t> default_sample_sizes;

    if (yaml_bench["default_min_test_time"].IsDefined())
        default_min_test_time_ms = yaml_bench["default_min_test_time"].as<decltype(default_min_test_time_ms)>();

    if (yaml_bench["default_sample_sizes"].IsDefined())
    {
        if (!yaml_bench["default_sample_sizes"].IsMap())
        {
            YAML::Mark yaml_mark = yaml_bench["default_sample_sizes"].Mark();
            std::stringstream ss;
            ss << "Value of field `default_sample_sizes` in ";
            if (yaml_mark.is_null())
                ss << "benchmark with ID " << benchmark_index;
            else
                ss << "line " << yaml_mark.line + 1;
            ss << " must be a map matching operation parameter index to the corresponding sample size.";
            throw std::runtime_error(ss.str());
        } // end if
        for (auto it = yaml_bench["default_sample_sizes"].begin(); it != yaml_bench["default_sample_sizes"].end(); ++it)
        {
            // retrieve the sample size in format `index: size`
            // this way, users can specify sample sizes for some, but not necessarily all operation parameters.
            std::size_t key = it->first.as<std::size_t>();
            if (default_sample_sizes.size() <= key)
                default_sample_sizes.resize(key + 1, 0);
            default_sample_sizes[key] = it->second.as<std::uint64_t>();
        } // end for
    } // end if

    if (yaml_bench["params"].IsDefined() && yaml_bench["params"].size() > 0)
    {
        // create a component counter to iterate over the parameters using from-to-step model
        std::vector<std::size_t> component_sizes(yaml_bench["params"].size());
        for (std::size_t param_i = 0; param_i < component_sizes.size(); ++param_i)
        {
            // validate node

            if (!yaml_bench["params"][param_i].IsDefined())
            {
                std::stringstream ss;
                ss << "workload parameter index " << param_i << " not found in benchmark with ID " << benchmark_index << ", defined at line " << yaml_bench.Mark().line + 1 << ".";
                throw std::runtime_error(ss.str());
            } // end if
            if (!yaml_bench["params"][param_i]["name"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"name\" not found for workload parameter index " << param_i << ", benchmark ID " << benchmark_index << ", defined at line " << yaml_bench.Mark().line + 1 << ".";
                throw std::runtime_error(ss.str());
            } // end if
            if (!yaml_bench["params"][param_i]["type"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"type\" not found for workload parameter index " << param_i << ", benchmark ID " << benchmark_index << ", defined at line " << yaml_bench.Mark().line + 1 << ".";
                throw std::runtime_error(ss.str());
            } // end if
            if (!yaml_bench["params"][param_i]["value"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"value\" not found for workload parameter index " << param_i << ", benchmark ID " << benchmark_index << ", defined at line " << yaml_bench.Mark().line + 1 << ".";
                throw std::runtime_error(ss.str());
            } // end if

            YAML::Node yaml_param_value = yaml_bench["params"][param_i]["value"];
            if (!yaml_param_value["from"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"from\" not found for \"value\" in workload parameter index " << param_i << ", benchmark ID " << benchmark_index << ", defined at line " << yaml_bench.Mark().line + 1 << ".";
                throw std::runtime_error(ss.str());
            } // end if
            if (!yaml_param_value["to"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"to\" not found for \"value\" in workload parameter index " << param_i << ", benchmark ID " << benchmark_index << ", defined at line " << yaml_bench.Mark().line + 1 << ".";
                throw std::runtime_error(ss.str());
            } // end if
            if (!yaml_param_value["step"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"step\" not found for \"value\" in workload parameter index " << param_i << ", benchmark ID " << benchmark_index << ", defined at line " << yaml_bench.Mark().line + 1 << ".";
                throw std::runtime_error(ss.str());
            } // end if

            // convert param from-to-step into counter
            std::string param_type = yaml_bench["params"][param_i]["type"].as<std::string>();
            std::transform(param_type.begin(), param_type.end(), param_type.begin(),
                           [](unsigned char c) { return std::tolower(c); });
            if (param_type == "uint64")
            {
                component_sizes[param_i] =
                    computeComponentSize<std::uint64_t>(yaml_param_value["from"],
                                                        yaml_param_value["to"],
                                                        yaml_param_value["step"]);
            } // end if
            else if (param_type == "int64")
            {
                component_sizes[param_i] =
                    computeComponentSize<std::int64_t>(yaml_param_value["from"],
                                                       yaml_param_value["to"],
                                                       yaml_param_value["step"]);
            } // end if
            else if (param_type == "float64")
            {
                component_sizes[param_i] =
                    computeComponentSize<double>(yaml_param_value["from"],
                                                 yaml_param_value["to"],
                                                 yaml_param_value["step"]);
            } // end if
            else
            {
                std::stringstream ss;
                ss << "Invalid \"type\" found in parameter index " << param_i
                   << ", benchmark ID " << benchmark_index << ": "
                   << yaml_bench["params"][param_i]["type"].as<std::string>() << ".";
                throw std::runtime_error(ss.str());
            } // end if
        } // end for

        hebench::Utilities::Math::ComponentCounter c_counter(component_sizes);

        do
        {
            const std::vector<std::size_t> &count = c_counter.getCount();
            hebench::TestHarness::BenchmarkDescription::Configuration config;
            config.default_min_test_time_ms     = default_min_test_time_ms <= 0 ? fallback_min_test_time : default_min_test_time_ms;
            config.fallback_default_sample_size = fallback_sample_size;
            config.default_sample_sizes         = default_sample_sizes;
            for (std::size_t param_i = 0; param_i < count.size(); ++param_i)
            {
                // fill out WorkloadParam struct
                hebench::APIBridge::WorkloadParam w_param;
                YAML::Node yaml_param       = yaml_bench["params"][param_i];
                YAML::Node yaml_param_value = yaml_param["value"];
                std::string param_type      = yaml_param["type"].as<std::string>();
                std::transform(param_type.begin(), param_type.end(), param_type.begin(),
                               [](unsigned char c) { return std::tolower(c); });

                if (param_type == "uint64")
                {
                    w_param.data_type = hebench::APIBridge::WorkloadParamType::UInt64;
                    w_param.u_param   = computeParamValue<decltype(w_param.u_param)>(count[param_i],
                                                                                   yaml_param_value["from"],
                                                                                   yaml_param_value["step"]);
                } // end if
                else if (param_type == "int64")
                {
                    w_param.data_type = hebench::APIBridge::WorkloadParamType::Int64;
                    w_param.i_param   = computeParamValue<decltype(w_param.i_param)>(count[param_i],
                                                                                   yaml_param_value["from"],
                                                                                   yaml_param_value["step"]);
                } // end if
                else if (param_type == "float64")
                {
                    w_param.data_type = hebench::APIBridge::WorkloadParamType::Float64;
                    w_param.f_param   = computeParamValue<decltype(w_param.f_param)>(count[param_i],
                                                                                   yaml_param_value["from"],
                                                                                   yaml_param_value["step"]);
                } // end if

                hebench::Utilities::copyString(w_param.name, HEBENCH_MAX_BUFFER_SIZE, yaml_param["name"].as<std::string>());
                config.w_params.push_back(w_param);
            } // end for

            // check if benchmark is supported with specified parameters
            try
            {
                hebench::TestHarness::IBenchmarkDescriptor::DescriptionToken::Ptr p_token =
                    engine.describeBenchmark(benchmark_index, config);
                if (!p_token)
                    throw std::runtime_error("Unable to match benchmark.");
                retval.emplace_back();
                retval.back().index         = benchmark_index;
                retval.back().configuration = p_token->getBenchmarkConfiguration();
            }
            catch (std::runtime_error &ex)
            {
                std::stringstream ss;
                ss << "Benchmark ID: " << benchmark_index << std::endl
                   << ex.what();
                throw std::runtime_error(ss.str());
            }
        } while (!c_counter.inc()); // repeat until we have done all possible parameters
    } // end if
    else
    {
        // add an empty parameter list for workload
        try
        {
            retval.emplace_back();
            retval.back().configuration.default_min_test_time_ms = default_min_test_time_ms;
            retval.back().configuration.default_sample_sizes     = default_sample_sizes;
            hebench::TestHarness::IBenchmarkDescriptor::DescriptionToken::Ptr p_token =
                engine.describeBenchmark(benchmark_index, retval.back().configuration);
            if (!p_token)
                throw std::runtime_error("Unable to match benchmark.");
            retval.back().index         = benchmark_index;
            retval.back().configuration = p_token->getBenchmarkConfiguration();
        }
        catch (std::runtime_error &ex)
        {
            std::stringstream ss;
            ss << "Benchmark ID: " << benchmark_index << std::endl
               << ex.what();
            throw std::runtime_error(ss.str());
        }
    } // end else

    return retval;
}

//--------------------------
// class ConfigExporterImpl
//--------------------------

YAML::Node ConfigExporterImpl::createWorkloadParamYAMLNode(const hebench::APIBridge::WorkloadParam &w_param)
{
    YAML::Node retval;
    retval["name"] = w_param.name;
    switch (w_param.data_type)
    {
    case hebench::APIBridge::WorkloadParamType::Int64:
        retval["type"]  = "Int64";
        retval["value"] = createWorkloadParamValueYAMLNode<decltype(w_param.i_param)>(w_param.i_param);
        break;

    case hebench::APIBridge::WorkloadParamType::UInt64:
        retval["type"]  = "UInt64";
        retval["value"] = createWorkloadParamValueYAMLNode<decltype(w_param.u_param)>(w_param.u_param);
        break;

    case hebench::APIBridge::WorkloadParamType::Float64:
        retval["type"]  = "Float64";
        retval["value"] = createWorkloadParamValueYAMLNode<decltype(w_param.f_param)>(w_param.f_param);
        break;

    default:
        throw std::invalid_argument("Invalid data type detected in 'w_param'.");
        break;
    } // end switch

    return retval;
}

void ConfigExporterImpl::exportBenchmarkRequest2YAML(YAML::Emitter &out,
                                                     const BenchmarkRequest &bench_req,
                                                     const hebench::TestHarness::BenchmarkDescription::Description &description)
{
    const hebench::TestHarness::BenchmarkDescription::Configuration &config = bench_req.configuration;
    std::filesystem::path path_bench_desc                                   = description.path;
    std::stringstream ss;
    auto path_it = path_bench_desc.begin();
    if (path_it == path_bench_desc.end())
        throw std::logic_error("Invalid benchmark description text.");
    bool b_first_it = true;
    for (++path_it; path_it != path_bench_desc.end(); ++path_it)
    {
        if (b_first_it)
            b_first_it = false;
        else
            ss << " | ";
        std::string s_tmp = *path_it;
        s_tmp.erase(0, s_tmp.find_first_not_of('\"'));
        s_tmp.erase(s_tmp.find_last_not_of('\"') + 1);
        ss << s_tmp;
    } // end for

    YAML::Node node_benchmark;
    out << YAML::Newline
        << YAML::Comment("Benchmark with workload parameters:") << YAML::Newline
        << YAML::Comment("  " + description.workload_name) << YAML::Newline
        << YAML::Comment("Descriptor:") << YAML::Newline
        << YAML::Comment("  " + ss.str());
    node_benchmark["ID"]                    = bench_req.index;
    node_benchmark["default_min_test_time"] = config.default_min_test_time_ms;
    if (!config.default_sample_sizes.empty())
    {
        for (std::size_t op_param_i = 0; op_param_i < config.default_sample_sizes.size(); ++op_param_i)
            node_benchmark["default_sample_sizes"].push_back(config.default_sample_sizes[op_param_i]);
        node_benchmark["default_sample_sizes"]["to_map"] = "yes"; // convert sequence to map to have the indices as keys
        node_benchmark["default_sample_sizes"].remove("to_map");
    } // end if
    if (!config.w_params.empty())
    {
        for (std::size_t param_i = 0; param_i < config.w_params.size(); ++param_i)
            node_benchmark["params"].push_back(createWorkloadParamYAMLNode(config.w_params[param_i]));
        node_benchmark["params"]["to_map"] = "yes"; // convert sequence to map to have the indices as keys
        node_benchmark["params"].remove("to_map");
    } // end if
    out << node_benchmark << YAML::Newline;
}

//------------------------------
// class BenchmarkConfiguration
//------------------------------

BenchmarkConfigurator::BenchmarkConfigurator(std::weak_ptr<hebench::TestHarness::Engine> wp_engine,
                                             const std::string s_backend) :
    m_wp_engine(wp_engine),
    m_s_backend(s_backend)
{
    std::shared_ptr<hebench::TestHarness::Engine> p_engine = wp_engine.lock();
    if (!p_engine)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid empty 'wp_engine'."));

    std::size_t count_benchmarks = p_engine->countBenchmarks();
    m_default_benchmarks.reserve(count_benchmarks);
    for (std::size_t bench_i = 0; bench_i < count_benchmarks; ++bench_i)
    {
        std::vector<std::vector<hebench::APIBridge::WorkloadParam>> w_params_set =
            p_engine->getDefaultWorkloadParams(bench_i);
        for (std::size_t w_params_i = 0; w_params_i < w_params_set.size(); ++w_params_i)
        {
            m_default_benchmarks.emplace_back();
            BenchmarkRequest &bench_description      = m_default_benchmarks.back();
            bench_description.configuration.w_params = std::move(w_params_set[w_params_i]);
            // obtain full description for the benchmark
            hebench::TestHarness::IBenchmarkDescriptor::DescriptionToken::Ptr p_token =
                p_engine->describeBenchmark(bench_i, bench_description.configuration);
            if (!p_token)
                throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error: no matching benchmark found for backend request."));
            // store the data needed to retrieve the benchmark
            bench_description.index         = bench_i;
            bench_description.configuration = p_token->getBenchmarkConfiguration();
            //bench_description.description = p_token->getDescription();
        } // end for
    } // end for
}

void BenchmarkConfigurator::saveConfiguration(const std::string &yaml_filename,
                                              const std::vector<BenchmarkRequest> &bench_requests,
                                              std::uint64_t random_seed) const
{
    std::shared_ptr<hebench::TestHarness::Engine> p_engine = m_wp_engine.lock();
    if (!p_engine)
        throw std::logic_error(IL_LOG_MSG_CLASS("Invalid internal state: engine object has been released."));

    std::filesystem::path path_backend = m_s_backend;

    YAML::Emitter out;

    std::stringstream ss;
    ss << "HEBench auto-generated benchmark selection and configuration file." << std::endl
       << std::endl;
    if (path_backend.has_filename())
        ss << "Generated from backend: " << std::endl
           << "  " << path_backend.filename() << std::endl
           << std::endl;
    ss << "Only benchmarks with their workload parameters specified here will run when" << std::endl
       << "this configuration file is used with Test Harness. This configuration only" << std::endl
       << "works for the backend used to generate this file (the backend)." << std::endl
       << std::endl
       << "Benchmark \"ID\" represents workload and benchmark descriptor options for the" << std::endl
       << "backend. Benchmark \"params\" are the configurable workload parameters." << std::endl
       << std::endl
       << "To use this file do any of the following:" << std::endl
       << "  - Add or remove benchmarks based on pre-existing configurations." << std::endl
       << "  - Modify the configuration and workload parameters for existing or added" << std::endl
       << "    benchmarks." << std::endl
       << std::endl
       << "Global configuration values, if present, are used when local values are not" << std::endl
       << "specified per benchmark." << std::endl
       << std::endl
       << "When adding new benchmarks:" << std::endl
       << "  The only benchmark IDs supported are those already existing in this" << std::endl
       << "  auto-generated file as reported by the backend. Any new benchmark added" << std::endl
       << "  must have the same number of workload parameters as those already existing" << std::endl
       << "  in this auto-generated file with the same benchmark ID." << std::endl
       << std::endl
       << "When modifying workload parameters:" << std::endl
       << "  Number of workload parameters, their type and name must match those of any" << std::endl
       << "  auto-generated benchmark with the same ID." << std::endl
       << "  Refer to workload and backend specifications for supported range of values" << std::endl
       << "  for each workload parameter. Invalid values will cause the benchmark to" << std::endl
       << "  fail during execution." << std::endl;
    out << YAML::Comment(ss.str()) << YAML::Newline;

    // output benchmark default configuration
    out << YAML::Newline;

    out << YAML::BeginMap;

    ss = std::stringstream();
    ss << "Default minimum test time in milliseconds. Latency tests specifying" << std::endl
       << "default test time and Offline tests will be repeated until this time" << std::endl
       << "has elapsed. Defaults to 0 if not present (Latency executes twice," << std::endl
       << "Offline executes once).";
    out << YAML::Comment(ss.str())
        << YAML::Key << "default_min_test_time" << YAML::Value << 0;
    out << YAML::Newline << YAML::Newline;

    ss = std::stringstream();
    ss << "Default sample size to use for operation parameters that support" << std::endl
       << "variable sample size in Offline category. Defaults to benchmark" << std::endl
       << "specific if not present.";
    out << YAML::Comment(ss.str())
        << YAML::Key << "default_sample_size" << YAML::Value << 0;
    out << YAML::Newline << YAML::Newline;

    ss = std::stringstream();
    ss << "Random seed to use when generating synthetic data for these benchmarks." << std::endl
       << "Type: unsigned int. Defaults to system time when not present.";
    out << YAML::Comment(ss.str())
        << YAML::Key << "random_seed" << YAML::Value << random_seed;
    out << YAML::Newline << YAML::Newline;

    out << YAML::EndMap;

    // output benchmark list

    out << YAML::BeginMap;
    out << YAML::Key << "benchmark" << YAML::Value
        << YAML::BeginSeq;

    for (std::size_t i = 0; i < bench_requests.size(); ++i)
    {
        hebench::TestHarness::IBenchmarkDescriptor::DescriptionToken::Ptr p_token =
            p_engine->describeBenchmark(bench_requests[i].index, bench_requests[i].configuration);
        if (!p_token)
        {
            ss = std::stringstream();
            ss << "No matching benchmark found for backend request " << i
               << " for backend benchmark index " << bench_requests[i].index << ".";
            throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
        } // end if
        BenchmarkRequest bench_request_corrected;
        bench_request_corrected.index         = bench_requests[i].index;
        bench_request_corrected.configuration = p_token->getBenchmarkConfiguration();
        ConfigExporterImpl::exportBenchmarkRequest2YAML(out, bench_request_corrected, p_token->getDescription());
    } // end for

    out << YAML::EndSeq
        << YAML::EndMap;

    hebench::Utilities::writeToFile(
        yaml_filename,
        [&out](std::ostream &os) {
            os << out.c_str();
        },
        false, false);
}

std::vector<BenchmarkRequest> BenchmarkConfigurator::loadConfiguration(const std::string &yaml_filename,
                                                                       std::uint64_t &random_seed) const
{
    std::vector<BenchmarkRequest> retval;
    std::uint64_t default_min_test_time_ms = 0;
    std::uint64_t default_sample_size      = 0;

    std::shared_ptr<hebench::TestHarness::Engine> p_engine = m_wp_engine.lock();
    if (!p_engine)
        throw std::logic_error(IL_LOG_MSG_CLASS("Invalid internal state: engine object has been released."));

    YAML::Node root = YAML::LoadFile(yaml_filename);

    if (!root["benchmark"].IsDefined())
        throw std::runtime_error("Map \"benchmark\" not found at root of configuration file.");

    // parse benchmark defaults
    if (root["default_min_test_time"].IsDefined())
        default_min_test_time_ms =
            root["default_min_test_time"].as<decltype(default_min_test_time_ms)>();
    if (root["default_sample_size"].IsDefined())
        default_sample_size =
            root["default_sample_size"].as<decltype(default_sample_size)>();
    if (root["random_seed"].IsDefined())
        random_seed =
            root["random_seed"].as<std::uint64_t>();

    root = root["benchmark"];
    if (!root.IsSequence())
        throw std::runtime_error("Value for map \"benchmark\" is not a valid YAML sequence.");
    retval.reserve(root.size());
    for (std::size_t i = 0; i < root.size(); ++i)
    {
        if (!root[i]["ID"].IsDefined())
            throw std::runtime_error("Field \"ID\" not found on benchmark.");
        std::size_t benchmark_index = root[i]["ID"].as<std::size_t>();
        std::vector<BenchmarkRequest> node_requests =
            ConfigImporterImpl::importYAML2BenchmarkRequest(benchmark_index,
                                                            root[i],
                                                            *p_engine,
                                                            default_min_test_time_ms,
                                                            default_sample_size);
        retval.insert(retval.end(), node_requests.begin(), node_requests.end());
    } // end for

    return retval;
}

} // namespace Utilities
} // namespace hebench
