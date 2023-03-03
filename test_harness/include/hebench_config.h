
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Config_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Config_H_0596d40a3cce4b108a81595c50eb286d

#include <cstdint>
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

class BenchmarkConfigBroker;

struct BenchmarkRequest
{
    std::size_t index;
    hebench::TestHarness::BenchmarkDescription::Configuration configuration;
};

struct BenchmarkSession
{
    std::uint64_t random_seed;
    std::vector<std::int8_t> data;
    std::vector<BenchmarkRequest> benchmark_requests;
};

class BenchmarkConfigLoader
{
private:
    IL_DECLARE_CLASS_NAME(BenchmarkConfigLoader)
public:
    class FriendKey
    {
    public:
        friend class BenchmarkConfigBroker;

    private:
        FriendKey() = default;
    };

    BenchmarkConfigLoader(const BenchmarkConfigLoader &)  = delete;
    BenchmarkConfigLoader(const BenchmarkConfigLoader &&) = delete;

    /**
     * @brief Loads a benchmark configuration from yaml data contained in a file.
     * @param[in] yaml_filename File name containing the configuration data in
     * yaml format.
     * @throws Instance of `std::exception` on error.
     * @return Pointer to created `BenchmarkConfigLoader` with valid loaded configuration
     * data to be used by a `BenchmarkConfigBroker` to parse and generate a configuration
     * for benchmark session.
     */
    static std::shared_ptr<BenchmarkConfigLoader> create(const std::string &yaml_filename,
                                                         std::uint64_t fallback_random_seed);

    std::uint64_t getDefaultMinTestTimeMs() const { return m_default_min_test_time_ms; }
    std::uint64_t getDefaultSampleSize() const { return m_default_sample_size; }
    /**
     * @brief Retrieves the file name used to load the yaml data.
     */
    const std::string &getFilename() const { return m_filename; }
    const std::vector<std::int8_t> &getInitData() const { return m_data; }
    /**
     * @brief Retrieves the configuration random seed.
     * @return The random seed to be used for this configuration as loaded from
     * the configuration file, or a system clock pre-generated seed if none was
     * specified in the configuration file.
     */
    std::uint64_t getRandomSeed() const { return m_random_seed; }

    const void *getYAMLContent(const FriendKey &) const { return m_yaml_content.get(); }

private:
    BenchmarkConfigLoader(const std::string &yaml_filename, std::uint64_t fallback_random_seed);

    std::string m_filename;
    std::uint64_t m_random_seed;
    std::uint64_t m_default_min_test_time_ms;
    std::uint64_t m_default_sample_size;
    std::vector<std::int8_t> m_data;
    std::shared_ptr<void> m_yaml_content;
};

/**
 * @brief Provides facilities to configure the selection of benchmarks to run
 * based on the supported workloads from a loaded backend.
 */
class BenchmarkConfigBroker
{
private:
    IL_DECLARE_CLASS_NAME(BenchmarkConfigBroker)

public:
    BenchmarkConfigBroker(std::weak_ptr<hebench::TestHarness::Engine> wp_engine,
                          std::uint64_t random_seed   = 0,
                          const std::string s_backend = std::string());

    void exportConfiguration(const std::string &yaml_filename,
                             const BenchmarkSession &bench_configs) const;
    /**
     * @brief Imports a benchmark configuration from a configurator loader.
     * @param[in] loader A BenchmarkConfigLoader initialized with the yaml information
     * from the desired configuration file.
     * @return The benchmark session listing all the benchmarks to run.
     */
    BenchmarkSession importConfiguration(const BenchmarkConfigLoader &loader) const;
    const BenchmarkSession &getDefaultConfiguration() const { return m_default_benchmarks; }

private:
    std::weak_ptr<hebench::TestHarness::Engine> m_wp_engine;
    std::string m_s_backend;
    BenchmarkSession m_default_benchmarks;
};

} // namespace Utilities
} // namespace hebench

#endif // defined _HEBench_Harness_Config_H_0596d40a3cce4b108a81595c50eb286d
