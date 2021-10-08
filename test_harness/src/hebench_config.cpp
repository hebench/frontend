
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

#include "../include/hebench_config.h"
#include "include/hebench_engine.h"
#include "include/hebench_ibenchmark.h"
#include "include/hebench_math_utils.h"
#include "include/hebench_utilities.h"

namespace hebench {
namespace Utilities {

class ConfigExporterImpl
{
public:
    static void exportBenchmarkRequest2YAML(YAML::Emitter &out,
                                            const hebench::TestHarness::BenchmarkRequest &bench_req,
                                            const hebench::TestHarness::Engine &engine,
                                            const hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig &default_bench_config);

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
    static void importYAML2BenchmarkRequest(hebench::TestHarness::BenchmarkRequest &bench_req,
                                            const YAML::Node &yaml_bench,
                                            const hebench::TestHarness::Engine &engine,
                                            const hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig &default_bench_config);

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

void ConfigImporterImpl::importYAML2BenchmarkRequest(TestHarness::BenchmarkRequest &bench_req,
                                                     const YAML::Node &yaml_bench,
                                                     const TestHarness::Engine &engine,
                                                     const hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig &default_bench_config)
{
    assert(yaml_bench["ID"].as<decltype(bench_req.benchmark_index)>() == bench_req.benchmark_index);

    if (!yaml_bench["params"].IsDefined())
    {
        std::stringstream ss;
        ss << "Field \"params\" not found for benchmark ID " << bench_req.benchmark_index << ".";
        throw std::runtime_error(ss.str());
    } // end if

    if (yaml_bench["params"].size() > 0)
    {
        // create a component counter to iterate over the parameters using from-to-step model
        std::vector<std::size_t> component_sizes(yaml_bench["params"].size());
        for (std::size_t param_i = 0; param_i < component_sizes.size(); ++param_i)
        {
            // validate node

            if (!yaml_bench["params"][param_i].IsDefined())
            {
                std::stringstream ss;
                ss << "Parameter index " << param_i << " not found in benchmark with ID " << bench_req.benchmark_index << ".";
                throw std::runtime_error(ss.str());
            } // end if
            if (!yaml_bench["params"][param_i]["name"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"name\" not found for parameter index " << param_i << ", benchmark ID " << bench_req.benchmark_index << ".";
                throw std::runtime_error(ss.str());
            } // end if
            if (!yaml_bench["params"][param_i]["type"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"type\" not found for parameter index " << param_i << ", benchmark ID " << bench_req.benchmark_index << ".";
                throw std::runtime_error(ss.str());
            } // end if
            if (!yaml_bench["params"][param_i]["value"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"value\" not found for parameter index " << param_i << ", benchmark ID " << bench_req.benchmark_index << ".";
                throw std::runtime_error(ss.str());
            } // end if

            YAML::Node yaml_param_value = yaml_bench["params"][param_i]["value"];
            if (!yaml_param_value["from"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"from\" not found for \"value\" in parameter index " << param_i << ", benchmark ID " << bench_req.benchmark_index << ".";
                throw std::runtime_error(ss.str());
            } // end if
            if (!yaml_param_value["to"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"to\" not found for \"value\" in parameter index " << param_i << ", benchmark ID " << bench_req.benchmark_index << ".";
                throw std::runtime_error(ss.str());
            } // end if
            if (!yaml_param_value["step"].IsDefined())
            {
                std::stringstream ss;
                ss << "Field \"step\" not found for \"value\" in parameter index " << param_i << ", benchmark ID " << bench_req.benchmark_index << ".";
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
                   << ", benchmark ID " << bench_req.benchmark_index << ": "
                   << yaml_bench["params"][param_i]["type"].as<std::string>() << ".";
                throw std::runtime_error(ss.str());
            } // end if
        } // end for

        hebench::Utilities::Math::ComponentCounter c_counter(component_sizes);

        do
        {
            const std::vector<std::size_t> &count = c_counter.getCount();
            bench_req.sets_w_params.emplace_back();
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
                bench_req.sets_w_params.back().push_back(w_param);
            } // end for

            // check if benchmark is supported with specified parameters
            try
            {
                engine.describeBenchmark(default_bench_config, bench_req.benchmark_index, bench_req.sets_w_params.back());
            }
            catch (std::runtime_error &ex)
            {
                std::stringstream ss;
                ss << "Benchmark ID: " << bench_req.benchmark_index << std::endl
                   << ex.what();
                throw std::runtime_error(ss.str());
            }
        } while (!c_counter.inc()); // repeat until we have done all possible parameters
    } // end if
    else
    {
        // add an empty parameter list for workload
        bench_req.sets_w_params.emplace_back();
        // check if benchmark is supported without parameters
        try
        {
            engine.describeBenchmark(default_bench_config, bench_req.benchmark_index, bench_req.sets_w_params.back());
        }
        catch (std::runtime_error &ex)
        {
            std::stringstream ss;
            ss << "Benchmark ID: " << bench_req.benchmark_index << std::endl
               << ex.what();
            throw std::runtime_error(ss.str());
        }
    } // end else
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
                                                     const hebench::TestHarness::BenchmarkRequest &bench_req,
                                                     const hebench::TestHarness::Engine &engine,
                                                     const hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig &default_bench_config)
{
    for (std::size_t param_set_i = 0; param_set_i < bench_req.sets_w_params.size(); ++param_set_i)
    {
        auto token_bd                         = engine.describeBenchmark(default_bench_config, bench_req.benchmark_index, bench_req.sets_w_params[param_set_i]);
        std::filesystem::path path_bench_desc = token_bd->description.path;
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
            << YAML::Comment("  " + token_bd->description.workload_name) << YAML::Newline
            << YAML::Comment("Descriptor:") << YAML::Newline
            << YAML::Comment("  " + ss.str());
        node_benchmark["ID"] = bench_req.benchmark_index;
        for (std::size_t param_i = 0; param_i < bench_req.sets_w_params[param_set_i].size(); ++param_i)
            node_benchmark["params"].push_back(createWorkloadParamYAMLNode(bench_req.sets_w_params[param_set_i][param_i]));
        node_benchmark["params"]["to_map"] = "yes"; // convert sequence to map to have the indices as keys
        node_benchmark["params"].remove("to_map");
        out << node_benchmark << YAML::Newline;
    } // end for
}

//------------------------------
// class BenchmarkConfiguration
//------------------------------

std::size_t BenchmarkConfiguration::countBenchmarks2Run(const std::vector<hebench::TestHarness::BenchmarkRequest> &bench_config)
{
    return std::accumulate(bench_config.begin(), bench_config.end(), static_cast<std::size_t>(0),
                           [](std::size_t acc, const hebench::TestHarness::BenchmarkRequest &br) -> std::size_t {
                               return acc + br.sets_w_params.size();
                           });
}

BenchmarkConfiguration::BenchmarkConfiguration(std::weak_ptr<hebench::TestHarness::Engine> wp_engine,
                                               const std::string s_backend) :
    m_wp_engine(wp_engine),
    m_s_backend(s_backend)
{
    std::shared_ptr<hebench::TestHarness::Engine> p_engine = wp_engine.lock();
    if (!p_engine)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid empty 'wp_engine'."));

    m_default_benchmarks.resize(p_engine->countBenchmarks());
    for (std::size_t bench_i = 0; bench_i < m_default_benchmarks.size(); ++bench_i)
    {
        m_default_benchmarks[bench_i].benchmark_index = bench_i;
        m_default_benchmarks[bench_i].sets_w_params =
            p_engine->getDefaultWorkloadParams(bench_i);
        if (m_default_benchmarks[bench_i].sets_w_params.empty())
            m_default_benchmarks[bench_i].sets_w_params.resize(1);
    } // end for
}

void BenchmarkConfiguration::saveConfiguration(const std::string &yaml_filename,
                                               const std::vector<hebench::TestHarness::BenchmarkRequest> &bench_config,
                                               const hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig &default_bench_config) const
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
       << "  - Modify the workload parameters for existing or added benchmarks." << std::endl
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
        << YAML::Key << "default_min_test_time" << YAML::Value << default_bench_config.default_min_test_time_ms;
    out << YAML::Newline << YAML::Newline;

    ss = std::stringstream();
    ss << "Default sample size to use for operation parameters that support" << std::endl
       << "variable sample size in Offline category. Defaults to benchmark" << std::endl
       << "specific if not present.";
    out << YAML::Comment(ss.str())
        << YAML::Key << "default_sample_size" << YAML::Value << default_bench_config.default_sample_size;
    out << YAML::Newline << YAML::Newline;

    ss = std::stringstream();
    ss << "Random seed to use when generating synthetic data for these benchmarks." << std::endl
       << "Type: unsigned int. Defaults to system time when not present.";
    out << YAML::Comment(ss.str())
        << YAML::Key << "random_seed" << YAML::Value << default_bench_config.random_seed;
    out << YAML::Newline << YAML::Newline;

    out << YAML::EndMap;

    // output benchmark list

    out << YAML::BeginMap;
    out << YAML::Key << "benchmark" << YAML::Value
        << YAML::BeginSeq;

    for (std::size_t i = 0; i < bench_config.size(); ++i)
        ConfigExporterImpl::exportBenchmarkRequest2YAML(out, bench_config[i], *p_engine, default_bench_config);

    out << YAML::EndSeq
        << YAML::EndMap;

    hebench::Utilities::writeToFile(
        yaml_filename,
        [&out](std::ostream &os) {
            os << out.c_str();
        },
        false, false);
}

std::vector<hebench::TestHarness::BenchmarkRequest> BenchmarkConfiguration::loadConfiguration(const std::string &yaml_filename,
                                                                                              hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig &default_bench_config) const
{
    std::vector<hebench::TestHarness::BenchmarkRequest> retval;
    std::unordered_map<std::size_t, std::size_t> map_bench_reqs; // maps ID to array index to avoid having to search array every time

    std::shared_ptr<hebench::TestHarness::Engine> p_engine = m_wp_engine.lock();
    if (!p_engine)
        throw std::logic_error(IL_LOG_MSG_CLASS("Invalid internal state: engine object has been released."));

    YAML::Node root = YAML::LoadFile(yaml_filename);

    if (!root["benchmark"].IsDefined())
        throw std::runtime_error("Map \"benchmark\" not found at root of configuration file.");

    // parse benchmark defaults
    if (root["default_min_test_time"].IsDefined())
        default_bench_config.default_min_test_time_ms =
            root["default_min_test_time"].as<decltype(default_bench_config.default_min_test_time_ms)>();
    if (root["default_sample_size"].IsDefined())
        default_bench_config.default_sample_size =
            root["default_sample_size"].as<decltype(default_bench_config.default_sample_size)>();
    if (root["random_seed"].IsDefined())
        default_bench_config.random_seed =
            root["random_seed"].as<decltype(default_bench_config.random_seed)>();

    root = root["benchmark"];
    if (!root.IsSequence())
        throw std::runtime_error("Value for map \"benchmark\" is not a valid YAML sequence.");
    for (std::size_t i = 0; i < root.size(); ++i)
    {
        if (!root[i]["ID"].IsDefined())
            throw std::runtime_error("Field \"ID\" not found on benchmark.");
        std::size_t id = root[i]["ID"].as<std::size_t>();
        // add benchmark requests for the same ID into the same structure
        if (map_bench_reqs.count(id) <= 0)
        {
            map_bench_reqs[id] = retval.size();
            retval.emplace_back();
            retval.back().benchmark_index = id;
        } // end if
        hebench::TestHarness::BenchmarkRequest &bench_req = retval[map_bench_reqs[id]];
        ConfigImporterImpl::importYAML2BenchmarkRequest(bench_req, root[i], *p_engine, default_bench_config);
    } // end for

    return retval;
}

} // namespace Utilities
} // namespace hebench
