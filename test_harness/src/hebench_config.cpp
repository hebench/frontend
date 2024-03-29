// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

#include <yaml-cpp/yaml.h>

#include "hebench/modules/general/include/hebench_math_utils.h"
#include "hebench/modules/general/include/hebench_utilities.h"
#include "include/hebench_benchmark_description.h"
#include "include/hebench_benchmark_factory.h"
#include "include/hebench_config.h"
#include "include/hebench_engine.h"
#include "include/hebench_ibenchmark.h"
#include "include/hebench_utilities_harness.h"

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
    static std::vector<BenchmarkRequest> importYAML2BenchmarkRequest(const std::filesystem::path &working_path,
                                                                     std::size_t benchmark_index,
                                                                     const YAML::Node &yaml_bench,
                                                                     const hebench::TestHarness::Engine &engine,
                                                                     std::uint64_t fallback_min_test_time,
                                                                     std::uint64_t fallback_sample_size);
    template <typename T>
    static T retrieveValue(const std::string &s_value);
    template <typename T>
    static T retrieveValue(const YAML::Node &node)
    {
        return retrieveValue<T>(node.as<std::string>());
    }

private:
    static std::string getCleanEnvVariableName(std::string s_var_name)
    {
        s_var_name.erase(0, s_var_name.find_first_not_of(" \t\n\r\f\v"));
        s_var_name.erase(s_var_name.find_last_not_of(" \t\n\r\f\v") + 1);
        return !s_var_name.empty() && s_var_name.front() == '$' ? s_var_name.substr(1) : std::string();
    }
    static std::string evaluateEnvVariable(const std::string &s_var_name)
    {
        std::string retval;
        if (!s_var_name.empty())
        {
            const char *c_value = std::getenv(s_var_name.c_str());
            if (c_value)
                retval = c_value;
        } // end if
        return retval;
    }

    template <typename T>
    static std::size_t computeComponentSize(const YAML::Node &node_from,
                                            const YAML::Node &node_to,
                                            const YAML::Node &node_step)
    {
        T param_step = retrieveValue<T>(node_step);
        if (param_step == static_cast<T>(0))
            param_step = std::numeric_limits<T>::max();
        T tmp_to   = retrieveValue<T>(node_to);
        T tmp_from = retrieveValue<T>(node_from);
        if (tmp_to < tmp_from)
            tmp_to = tmp_from;
        return static_cast<std::size_t>((tmp_to - tmp_from) / param_step) + 1;
    }

    template <typename T>
    static T computeParamValue(std::size_t count,
                               const YAML::Node &node_from,
                               const YAML::Node &node_step)
    {
        return static_cast<T>(count * retrieveValue<T>(node_step) + retrieveValue<T>(node_from));
    }
};

//--------------------------
// class ConfigImporterImpl
//--------------------------

template <>
inline std::string ConfigImporterImpl::retrieveValue<std::string>(const std::string &s_value)
{
    std::string s_tmp = getCleanEnvVariableName(s_value);
    return s_tmp.empty() ? s_value : evaluateEnvVariable(s_tmp);
}

template <typename T>
inline T ConfigImporterImpl::retrieveValue(const std::string &s_value)
{
    T retval;
    std::string s_tmp = getCleanEnvVariableName(s_value);
    std::stringstream ss(s_tmp.empty() ? s_value : evaluateEnvVariable(s_tmp));
    ss >> retval;
    if (!ss)
    {
        std::stringstream ss_err;
        ss_err << "ConfigImporterImpl::" << __func__ << "():" << __LINE__ << ": "
               << "Invalid conversion for value \"" << s_value << "\"";
        throw std::runtime_error(ss_err.str());
    } // end if

    return retval;
}

std::vector<BenchmarkRequest> ConfigImporterImpl::importYAML2BenchmarkRequest(const std::filesystem::path &working_path,
                                                                              std::size_t benchmark_index,
                                                                              const YAML::Node &yaml_bench,
                                                                              const TestHarness::Engine &engine,
                                                                              std::uint64_t fallback_min_test_time,
                                                                              std::uint64_t fallback_sample_size)
{
    assert(retrieveValue<decltype(benchmark_index)>(yaml_bench["ID"]) == benchmark_index);

    std::vector<BenchmarkRequest> retval;

    std::filesystem::path dataset_filename;
    std::uint64_t default_min_test_time_ms = 0;
    std::vector<std::uint64_t> default_sample_sizes;

    if (yaml_bench["dataset"].IsDefined() && !yaml_bench["dataset"].IsNull())
    {
        dataset_filename = std::filesystem::path(retrieveValue<std::string>(yaml_bench["dataset"]));
        if (dataset_filename.is_relative())
            dataset_filename = working_path / dataset_filename;
        try
        {
            // may throw if file does not exist
            dataset_filename = std::filesystem::canonical(dataset_filename);
        }
        catch (const std::exception &ex)
        {
            YAML::Mark yaml_mark = yaml_bench["dataset"].Mark();
            std::stringstream ss;
            ss << "Error in ";
            if (yaml_mark.is_null())
                ss << "benchmark with ID " << benchmark_index;
            else
                ss << "line " << yaml_mark.line + 1;
            ss << "." << std::endl
               << "  dataset: \"" << yaml_bench["dataset"] << "\"" << std::endl
               << ex.what();
            throw std::runtime_error(ss.str());
        }
    } // end if

    if (yaml_bench["default_min_test_time"].IsDefined())
        default_min_test_time_ms = retrieveValue<decltype(default_min_test_time_ms)>(yaml_bench["default_min_test_time"]);

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
            default_sample_sizes[key] = retrieveValue<std::uint64_t>(it->second);
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
            std::string param_type = hebench::Utilities::ToLowerCase(retrieveValue<std::string>(yaml_bench["params"][param_i]["type"]));
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
            config.dataset_filename             = dataset_filename.string();
            config.default_min_test_time_ms     = default_min_test_time_ms <= 0 ? fallback_min_test_time : default_min_test_time_ms;
            config.fallback_default_sample_size = fallback_sample_size;
            config.default_sample_sizes         = default_sample_sizes;
            for (std::size_t param_i = 0; param_i < count.size(); ++param_i)
            {
                // fill out WorkloadParam struct
                hebench::APIBridge::WorkloadParam w_param;
                YAML::Node yaml_param       = yaml_bench["params"][param_i];
                YAML::Node yaml_param_value = yaml_param["value"];
                std::string param_type      = hebench::Utilities::ToLowerCase(retrieveValue<std::string>(yaml_param["type"]));

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

                hebench::Utilities::copyString(w_param.name, HEBENCH_MAX_BUFFER_SIZE, retrieveValue<std::string>(yaml_param["name"]));
                config.w_params.push_back(w_param);
            } // end for

            // check if benchmark is supported with specified parameters
            try
            {
                if (!retval.empty() && !config.dataset_filename.empty())
                    throw std::runtime_error("External dataset is only supported on benchmark configuration blocks that define a single benchmark "
                                             "(for a workload parameter, fields `from` and `to` must match).");
                hebench::TestHarness::IBenchmarkDescriptor::DescriptionToken::Ptr p_token =
                    engine.describeBenchmark(benchmark_index, config);
                if (!p_token)
                    throw std::runtime_error("Unable to match benchmark: no match found able to support selected benchmark with specified workload parameters.");
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
    std::stringstream ss;

    ss << "Section \"description\" is for informational purposes only. It shows the" << std::endl
       << "benchmark descriptor matching the benchmark ID. Changing contents of" << std::endl
       << "\"description\" has no effect.";
    out << YAML::Newline;
    out << YAML::Comment(ss.str());

    YAML::Node node_benchmark;
    YAML::Node node_bench_description;
    node_bench_description["workload"]      = description.workload;
    node_bench_description["workload_name"] = description.workload_name;
    node_bench_description["data_type"]     = description.data_type;
    node_bench_description["category"]      = description.category;
    node_bench_description["scheme"]        = description.scheme;
    node_bench_description["security"]      = description.security;
    node_bench_description["cipher_flags"]  = description.cipher_flags;
    node_bench_description["other"]         = description.other;
    node_bench_description["notes"]         = YAML::Node(YAML::NodeType::Null);
    node_benchmark["ID"]                    = bench_req.index;
    node_benchmark["description"]           = node_bench_description;
    node_benchmark["dataset"]               = YAML::Node(YAML::NodeType::Null);
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

//-----------------------------
// class BenchmarkConfigLoader
//-----------------------------

std::shared_ptr<BenchmarkConfigLoader> BenchmarkConfigLoader::create(const std::string &yaml_filename,
                                                                     std::uint64_t fallback_random_seed)
{
    std::shared_ptr<BenchmarkConfigLoader> p_retval =
        std::shared_ptr<BenchmarkConfigLoader>(new BenchmarkConfigLoader(yaml_filename, fallback_random_seed));
    return p_retval;
}

BenchmarkConfigLoader::BenchmarkConfigLoader(const std::string &yaml_filename,
                                             std::uint64_t fallback_random_seed) :
    m_random_seed(fallback_random_seed),
    m_default_min_test_time_ms(0),
    m_default_sample_size(0)
{
    // get absolute path and directory of yaml file to load
    m_filename                     = std::filesystem::canonical(yaml_filename).string();
    std::filesystem::path file_dir = std::filesystem::path(m_filename).remove_filename();
    YAML::Node root                = YAML::LoadFile(m_filename);

    if (!root["benchmark"].IsDefined())
        throw std::runtime_error("Map \"benchmark\" not found at root of configuration file.");
    if (!root["benchmark"].IsSequence())
        throw std::runtime_error("Value for map \"benchmark\" is not a valid YAML sequence.");

    // parse benchmark defaults
    if (root["default_min_test_time"].IsDefined())
        m_default_min_test_time_ms =
            ConfigImporterImpl::retrieveValue<decltype(m_default_min_test_time_ms)>(root["default_min_test_time"]);
    if (root["default_sample_size"].IsDefined())
        m_default_sample_size =
            ConfigImporterImpl::retrieveValue<decltype(m_default_sample_size)>(root["default_sample_size"]);
    if (root["random_seed"].IsDefined())
        m_random_seed = ConfigImporterImpl::retrieveValue<std::uint64_t>(root["random_seed"]);

    if (root["initialization_data"].IsDefined() && !root["initialization_data"].IsNull())
    {
        // process initialization data
        std::string init_data = ConfigImporterImpl::retrieveValue<std::string>(root["initialization_data"]);
        if (!init_data.empty())
        {
            std::filesystem::path init_data_filepath = init_data;
            // check if this is a relative file name
            if (init_data_filepath.is_relative())
                // filenames are relative to the yaml file
                init_data_filepath = file_dir / init_data_filepath;
            if (std::filesystem::exists(init_data_filepath) && std::filesystem::is_regular_file(init_data_filepath))
            {
                std::ifstream fnum;
                fnum.open(init_data_filepath, std::ifstream::in | std::ifstream::binary);
                if (!fnum.is_open())
                {
                    std::stringstream ss;
                    ss << "Could not open file " << init_data_filepath
                       << " for reading as indicated by field `initialization_data`.";
                    throw std::runtime_error(ss.str());
                } // end if
                // read the whole file
                fnum.seekg(0, fnum.end);
                m_data.resize(fnum.tellg());
                fnum.seekg(0, fnum.beg);
                fnum.read(reinterpret_cast<std::ifstream::char_type *>(m_data.data()), m_data.size());
            } // end if
            else
                // make the actual yaml string into the initialization data
                m_data.assign(init_data.begin(), init_data.end());
        } // end if
    } // end if

    m_yaml_content = std::shared_ptr<YAML::Node>(new YAML::Node(root));
}

//-----------------------------
// class BenchmarkConfigBroker
//-----------------------------

BenchmarkConfigBroker::BenchmarkConfigBroker(std::weak_ptr<hebench::TestHarness::Engine> wp_engine,
                                             std::uint64_t random_seed,
                                             const std::string s_backend) :
    m_wp_engine(wp_engine),
    m_s_backend(s_backend)
{
    std::shared_ptr<hebench::TestHarness::Engine> p_engine = wp_engine.lock();
    if (!p_engine)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid empty 'wp_engine'."));

    std::size_t count_benchmarks                      = p_engine->countBenchmarks();
    m_default_benchmarks.random_seed                  = random_seed;
    std::vector<BenchmarkRequest> &benchmark_requests = m_default_benchmarks.benchmark_requests;
    benchmark_requests.reserve(count_benchmarks);
    for (std::size_t bench_i = 0; bench_i < count_benchmarks; ++bench_i)
    {
        std::vector<std::vector<hebench::APIBridge::WorkloadParam>> w_params_set =
            p_engine->getDefaultWorkloadParams(bench_i);
        for (std::size_t w_params_i = 0; w_params_i < w_params_set.size(); ++w_params_i)
        {
            benchmark_requests.emplace_back();
            BenchmarkRequest &bench_description      = benchmark_requests.back();
            bench_description.configuration.w_params = std::move(w_params_set[w_params_i]);
            // obtain full description for the benchmark
            hebench::TestHarness::IBenchmarkDescriptor::DescriptionToken::Ptr p_token =
                p_engine->describeBenchmark(bench_i, bench_description.configuration);
            if (!p_token)
                throw std::runtime_error(IL_LOG_MSG_CLASS("Unexpected error: no matching benchmark found for backend request."));
            // store the data needed to retrieve the benchmark
            bench_description.index         = bench_i;
            bench_description.configuration = p_token->getBenchmarkConfiguration();
        } // end for
    } // end for
}

void BenchmarkConfigBroker::exportConfiguration(const std::string &yaml_filename,
                                                const BenchmarkSession &bench_configs) const
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
       << "  fail during execution." << std::endl
       << std::endl
       << "If non-null \"dataset\" is specified for a benchmark, the framework will" << std::endl
       << "attempt to load the specified file and use its contents as values for inputs" << std::endl
       << "and ground truths instead of using synthetic data. For a benchmark" << std::endl
       << "description specifying a dataset file, all workload parameter ranges must" << std::endl
       << "resolve to single values." << std::endl;

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
        << YAML::Key << "random_seed" << YAML::Value << bench_configs.random_seed;
    out << YAML::Newline << YAML::Newline;

    ss = std::stringstream();
    ss << "Optional backend initialization data." << std::endl
       << "Type: string (can be null)." << std::endl
       << "When present, if this is the name of an existing file (relative to this" << std::endl
       << "file, or absolute), the file binary contents will be forwarded to the" << std::endl
       << "backend engine during initialization. Otherwise, the specified string is" << std::endl
       << "forwarded as is (without null terminator)";
    out << YAML::Comment(ss.str())
        << YAML::Key << "initialization_data" << YAML::Value << YAML::Null;
    out << YAML::Newline << YAML::Newline;

    out << YAML::EndMap;

    // output benchmark list

    out << YAML::BeginMap;
    out << YAML::Key << "benchmark" << YAML::Value
        << YAML::BeginSeq;

    auto &bench_requests = bench_configs.benchmark_requests;

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
        BenchmarkRequest bench_request_corrected; // finalize the benchmark request to output default values
        bench_request_corrected.index                                  = bench_requests[i].index;
        bench_request_corrected.configuration                          = p_token->getBenchmarkConfiguration();
        bench_request_corrected.configuration.default_min_test_time_ms = p_token->getBackendDescription().descriptor.cat_params.min_test_time_ms;
        hebench::APIBridge::Category category                          = p_token->getBackendDescription().descriptor.category;
        auto &sample_sizes                                             = bench_request_corrected.configuration.default_sample_sizes;
        assert(sample_sizes.size() < HEBENCH_MAX_OP_PARAMS);
        switch (category)
        {
        case hebench::APIBridge::Category::Latency:
            // no sample sizes for latency test
            sample_sizes.clear();
            break;
        case hebench::APIBridge::Category::Offline:
            for (std::size_t i = 0; i < sample_sizes.size(); ++i)
                sample_sizes[i] = p_token->getBackendDescription().descriptor.cat_params.offline.data_count[i];
            break;
        default:
            break;
        } // end switch
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

BenchmarkSession BenchmarkConfigBroker::importConfiguration(const BenchmarkConfigLoader &loader) const
{
    BenchmarkSession retval;
    std::uint64_t default_min_test_time_ms = 0;
    std::uint64_t default_sample_size      = 0;

    const void *p_yaml_content = loader.getYAMLContent({});
    if (!p_yaml_content)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid loader: internal YAML content is not initialized."));
    std::shared_ptr<hebench::TestHarness::Engine> p_engine = m_wp_engine.lock();
    if (!p_engine)
        throw std::logic_error(IL_LOG_MSG_CLASS("Invalid internal state: engine object has been released."));

    std::filesystem::path file_dir = std::filesystem::path(loader.getFilename()).remove_filename();
    YAML::Node root                = *reinterpret_cast<const YAML::Node *>(p_yaml_content);

    // Root is already validated by loader to be a valid benchmark sequence
    // (individual benchmarks are not validated by loader).
    root                                              = root["benchmark"];
    std::vector<BenchmarkRequest> &benchmark_requests = retval.benchmark_requests;
    benchmark_requests.reserve(root.size());
    for (std::size_t i = 0; i < root.size(); ++i)
    {
        if (!root[i]["ID"].IsDefined())
            throw std::runtime_error("Field \"ID\" not found in benchmark.");
        std::size_t benchmark_index = ConfigImporterImpl::retrieveValue<std::size_t>(root[i]["ID"]);
        std::vector<BenchmarkRequest> node_requests =
            ConfigImporterImpl::importYAML2BenchmarkRequest(file_dir,
                                                            benchmark_index,
                                                            root[i],
                                                            *p_engine,
                                                            default_min_test_time_ms,
                                                            default_sample_size);
        benchmark_requests.insert(benchmark_requests.end(), node_requests.begin(), node_requests.end());
    } // end for

    retval.random_seed = loader.getRandomSeed();
    retval.data        = loader.getInitData();

    return retval;
}

} // namespace Utilities
} // namespace hebench
