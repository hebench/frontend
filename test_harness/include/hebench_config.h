
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Config_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Config_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "hebench/modules/logging/include/logging.h"
#include "hebench_benchmark_description.h"
#include "include/hebench_ibenchmark.h"

namespace hebench {
namespace TestHarness {
class Engine;
}
} // namespace hebench

namespace hebench {
namespace Utilities {

struct BenchmarkRequest
{
    std::size_t index;
    hebench::TestHarness::BenchmarkDescription::Configuration configuration;
};

/**
 * @brief Provides facilities to configure the selection of benchmarks to run
 * based on the supported workloads from a loaded backend.
 */
class BenchmarkConfigurator
{
private:
    IL_DECLARE_CLASS_NAME(BenchmarkConfigurator)

public:
    BenchmarkConfigurator(std::weak_ptr<hebench::TestHarness::Engine> wp_engine,
                          const std::string s_backend = std::string());

    void saveConfiguration(const std::string &yaml_filename,
                           const std::vector<BenchmarkRequest> &bench_configs,
                           std::uint64_t random_seed) const;
    /**
     * @brief Loads a configuration from a yaml file.
     * @param[in] yaml_filename
     * @param random_seed Reference where to store the random seed to use for the run.
     * If no seed is specified in the configuration file, the value already in this
     * variable is not modified.
     * @return A list of all the benchmarks to run.
     */
    std::vector<BenchmarkRequest> loadConfiguration(const std::string &yaml_filename,
                                                    std::uint64_t &random_seed) const;
    const std::vector<BenchmarkRequest> &getDefaultConfiguration() const { return m_default_benchmarks; }

private:
    std::weak_ptr<hebench::TestHarness::Engine> m_wp_engine;
    std::string m_s_backend;
    std::vector<BenchmarkRequest> m_default_benchmarks;
};

} // namespace Utilities
} // namespace hebench

#endif // defined _HEBench_Harness_Config_H_0596d40a3cce4b108a81595c50eb286d
