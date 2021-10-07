
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Config_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Config_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "hebench_types_harness.h"
#include "include/hebench_ibenchmark.h"
#include "modules/logging/include/logging.h"

namespace hebench {
namespace TestHarness {
class Engine;
}
} // namespace hebench

namespace hebench {
namespace Utilities {

/**
 * @brief Provides facilities to configure the selection of benchmarks to run
 * based on the supported workloads from a loaded backend.
 */
class BenchmarkConfiguration
{
private:
    IL_DECLARE_CLASS_NAME(BenchmarkConfiguration)

public:
    static std::size_t countBenchmarks2Run(const std::vector<hebench::TestHarness::BenchmarkRequest> &bench_config);
    BenchmarkConfiguration(std::weak_ptr<hebench::TestHarness::Engine> wp_engine,
                           const std::string s_backend = std::string());

    void saveConfiguration(const std::string &yaml_filename,
                           const std::vector<hebench::TestHarness::BenchmarkRequest> &bench_config,
                           const hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig &default_bench_config) const;
    std::vector<hebench::TestHarness::BenchmarkRequest> loadConfiguration(const std::string &yaml_filename,
                                                                          hebench::TestHarness::IBenchmarkDescription::BenchmarkConfig &default_bench_config) const;
    const std::vector<hebench::TestHarness::BenchmarkRequest> &getDefaultConfiguration() const { return m_default_benchmarks; }

private:
    std::weak_ptr<hebench::TestHarness::Engine> m_wp_engine;
    std::string m_s_backend;
    std::vector<hebench::TestHarness::BenchmarkRequest> m_default_benchmarks;
};

} // namespace Utilities
} // namespace hebench

#endif // defined _HEBench_Harness_Config_H_0596d40a3cce4b108a81595c50eb286d
