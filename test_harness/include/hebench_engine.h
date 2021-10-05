
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Engine_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Engine_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>
#include <string>
#include <vector>

#include "hebench/api_bridge/types.h"
#include "modules/general/include/nocopy.h"
#include "modules/logging/include/logging.h"

#include "hebench_benchmark_factory.h"

namespace hebench {
namespace TestHarness {

class Engine : public std::enable_shared_from_this<Engine>
{
public:
    DISABLE_COPY(Engine)
    DISABLE_MOVE(Engine)
private:
    IL_DECLARE_CLASS_NAME(Engine)

public:
    typedef std::shared_ptr<Engine> Ptr;

    static std::string getErrorDescription(hebench::APIBridge::ErrorCode err_code);
    void validateRetCode(hebench::APIBridge::ErrorCode err_code, bool last_error = true) const;
    static Engine::Ptr create();

    virtual ~Engine();

    std::string getSchemeName(hebench::APIBridge::Scheme s) const;
    std::string getSecurityName(hebench::APIBridge::Scheme s,
                                hebench::APIBridge::Security sec) const;
    //    std::string getExtraDescription(const hebench::APIBridge::BenchmarkDescriptor &p_bench_desc,
    //                                    const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const;
    std::string getExtraDescription(hebench::APIBridge::Handle h_bench_desc,
                                    const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const;

    /**
     * @brief Number of benchmarks for which back-end is registered to perform.
     */
    std::size_t countBenchmarks() const { return m_h_bench_desc.size(); }

    /**
     * @brief Creates the the benchmark workload represented by the specified token.
     * @param[in] p_token Token of the described benchmark workload as returned by
     * describeBenchmark(). Cannot be `null`.
     * @return Smart pointer to the created benchmark workload.
     * @throws std::logic_error when creating a new benchmark without destroying
     * previously existing benchmark.
     * @throws std::invalid_argument on invalid \p p_token.
     * @details As long as the returned benchmark exists, this engine cannot
     * be destroyed. This method ensures that only one benchmark from this engine exists
     * at a time.
     *
     * The returned object is to be used by Test Harness to execute the benchmark.
     */
    IBenchmark::Ptr createBenchmark(BenchmarkFactory::BenchmarkToken::Ptr p_token,
                                    hebench::Utilities::TimingReportEx &out_report);
    /**
     * @brief Describes a benchmark workload that matches the specified description
     * from the benchmarks registered by the back-end.
     * @param[in] bench_config Configuration for the benchmark to describe.
     * @param[in] index Index of the registered back-end benchmark description for
     * which to create a benchmark workload.
     * @param[in] w_params Parameters for the workload to be executed by the benchmark.
     * This will be empty if benchmark does not support parameters.
     * @return A token describing the benchmark to be executed given the workload
     * and parameters.
     * @throws Instance of std::exception on errors.
     * @details The token returned by this method is to be used in a call to
     * createBenchmark() to instantiate the actual benchmark to run.
     */
    BenchmarkFactory::BenchmarkToken::Ptr describeBenchmark(const IBenchmarkDescription::BenchmarkConfig &bench_config,
                                                            std::size_t index,
                                                            const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const;

    /**
     * @brief Retrieves the list of default parameters for a workload as specified
     * by the back-end.
     * @param[in] index Index of the benchmark to query for default parameters.
     * @return A vector with the list of default arguments for the parameters
     * of the workload. Vector is empty if workload does not support parameters.
     */
    std::vector<std::vector<hebench::APIBridge::WorkloadParam>> getDefaultWorkloadParams(std::size_t index) const;

    std::string getLastErrorDescription() const;

    const hebench::APIBridge::Handle &handle() const { return m_handle; }

private:
    hebench::APIBridge::Handle m_handle;
    // handles to back-end registered benchmark descriptors in order of subscription
    std::vector<hebench::APIBridge::Handle> m_h_bench_desc;
    std::weak_ptr<IBenchmark> m_last_benchmark; // keeps track of whether a benchmark is already created

    Engine();
    void init();
};

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_Engine_H_0596d40a3cce4b108a81595c50eb286d
