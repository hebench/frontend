
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_BenchmarkDescription_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_BenchmarkDescription_H_0596d40a3cce4b108a81595c50eb286d

#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <ratio>
#include <string>
#include <unordered_map>
#include <vector>

#include "hebench/api_bridge/types.h"

namespace hebench {
namespace TestHarness {

class Engine;

namespace BenchmarkDescription {

class Backend
{
private:
    struct Internal
    {
        std::size_t m_index;
        hebench::APIBridge::Handle m_handle;
        hebench::APIBridge::BenchmarkDescriptor m_descriptor;
        std::size_t m_operation_params_count;
    };
    Internal m_backend_description;

public:
    friend class hebench::TestHarness::Engine;

    /**
     * @brief Index of the benchmark as registered by backend.
     */
    const std::size_t &index;
    /**
     * @brief Handle to the benchmark description as registered by backend.
     */
    const hebench::APIBridge::Handle &handle;
    /**
     * @brief Benchmark backend descriptor, as retrieved by backend,
     * corresponding to the registration handle `h_desc`.
     */
    const hebench::APIBridge::BenchmarkDescriptor &descriptor;
    /**
     * @brief Number of operation parameters for the specified workload.
     */
    std::size_t &operation_params_count;

    Backend() :
        Backend(true)
    {
    }
    Backend(const Backend &src) :
        Backend(false)
    {
        m_backend_description = src.m_backend_description;
    }
    virtual ~Backend() {}
    Backend &operator=(const Backend &src)
    {
        if (&src != this)
        {
            this->m_backend_description = src.m_backend_description;
        } // end if
        return *this;
    }

private:
    Backend(bool init) :
        m_backend_description(),
        index(m_backend_description.m_index),
        handle(m_backend_description.m_handle),
        descriptor(m_backend_description.m_descriptor),
        operation_params_count(m_backend_description.m_operation_params_count)
    {
        if (init)
            std::memset(&m_backend_description, 0, sizeof(m_backend_description));
    }
};

/**
 * @brief Specifies a benchmark configuration.
 * @details Parameters are established by priority based on where they are
 * specified first. The priority is:
 *
 * 1. backend specified
 * 2. benchmark specific in config file
 * 3. global config file
 * 4. workload specification
 *
 * If any of these is `0`, then the next down the list is used. Workload specification
 * must always be greater than `0`. Workload specification sizes are always supported.
 * Sample sizes from any other sources are requests and may be satisfied based on
 * actual dataset size.
 */
class Configuration
{
public:
    Configuration() :
        default_min_test_time_ms(0),
        fallback_default_sample_size(0),
        b_single_path_report(false),
        time_unit(0)
    {
    }

    /**
     * @brief Default minimum test time in milliseconds.
     * @details This will be the default time used when backends specify 0
     * for their minimum test time.
     */
    std::uint64_t default_min_test_time_ms;
    /**
     * @brief Default sample size to be used if a specific size is not specified
     * in the `default_sample_sizes` collection. If this is 0, then, the workload
     * default should be used.
     */
    std::uint64_t fallback_default_sample_size;
    /**
     * @brief File containing data for the benchmark. If empty string, benchmarks
     * that can auto generate the dataset will do so.
     * @details Set to a filename containing the data to be used as input (and,
     * optionally, ground truth output) for the benchmark. The benchmark receiving
     * this configuration will try to load the external data by attempting to match
     * the file contents to a known format.
     *
     * Some benchmarks do not support external datasets, others require them, yet
     * others support both. See the particular workload definition documentation
     * for information on configuration feature support.
     */
    std::string dataset_filename;
    /**
     * @brief Default sample size for each operation parameter.
     * @details If the number of elements in this array is less than the number of
     * operation parameters, all missing sizes are expected to be default. On the
     * other hand, If the number of elements is greater, extra values should be ignored.
     */
    std::vector<std::uint64_t> default_sample_sizes;
    /**
     * @brief Set of arguments for workload parameters.
     */
    std::vector<hebench::APIBridge::WorkloadParam> w_params;
    /**
     * @brief Defines if the workload report will be created in a single-level directory
     */
    bool b_single_path_report;
    /**
     * @brief Specifies the time unit for the summary report for this benchmark.
     * @details The time unit is one of `0`, `'s'`, `'m'`, `'u'`, `'n'`, corresponding
     * to the time units defined in hebench::ReportGen::cpp::TimingPrefixUtility .
     * @sa hebench::ReportGen::cpp::TimingPrefixUtility
     */
    char time_unit;
};

class Description
{
public:
    /**
     * @brief Human-readable friendly name of the benchmark workload.
     */
    std::string workload_name;
    /**
     * @brief CSV formatted header for this benchmark. This will be the header
     * pre-pended to the report containing the benchmark results.
     */
    std::string header;
    /**
     * @brief A string uniquely representing the benchmark descriptor that
     * can be used as a relative directory path. This may be several directories deep.
     */
    std::string path;
};

} // namespace BenchmarkDescription
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_BenchmarkDescription_H_0596d40a3cce4b108a81595c50eb286d
