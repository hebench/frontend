
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Benchmark_Factory_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Benchmark_Factory_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>
#include <string>
#include <vector>

#include "modules/general/include/nocopy.h"
#include "modules/logging/include/logging.h"

#include "hebench/api_bridge/types.h"
#include "hebench_ibenchmark.h"
#include "hebench_utilities.h"

namespace hebench {
namespace TestHarness {

class Engine;

class BenchmarkFactory final
{
public:
    DISABLE_COPY(BenchmarkFactory)
    DISABLE_MOVE(BenchmarkFactory)
private:
    IL_DECLARE_CLASS_NAME(BenchmarkFactory)
public:
    friend class Engine;

    /**
     * @brief Returns a token representing a benchmark that can perform the described
     * workload with specified parameters, if any.
     * @param[in] engine Engine requesting the matching.
     * @param[in] backend_desc Backend descriptor to match.
     * @param[in] config Configuration of benchmark to match.
     * @returns A valid token representing the matched benchmark.
     * @returns `null` if no match was found.
     */
    static IBenchmarkDescriptor::DescriptionToken::Ptr matchBenchmarkDescriptor(const Engine &engine,
                                                                                const BenchmarkDescription::Backend &backend_desc,
                                                                                const BenchmarkDescription::Configuration &config);
    /**
     * @brief Registers a benchmark description object that represents one of the
     * supported workloads.
     * @param[in] p_desc_obj Smart pointer to benchmark description object to register.
     * @returns `true` if registration is successful.
     * @returns `false` otherwise.
     * @details
     * This method should be called for each benchmark during static initialization to
     * register the benchmark description before the application main entry-point is reached.
     *
     * This method does not throw exceptions.
     */
    static bool registerSupportedBenchmark(std::shared_ptr<IBenchmarkDescriptor> p_desc_obj);

private:
    static std::vector<std::shared_ptr<IBenchmarkDescriptor>> &getRegisteredBenchmarks();
    /**
     * @brief Instantiates the benchmark represented by the specified token.
     * @param p_engine Calling engine object.
     * @param[in] p_token Token representing the benchmark to instantiate, as returned by
     * matchBenchmarkDescriptor().
     * @return The benchmark object for which test harness can issue calls to
     * execute the benchmark.
     * @throws std::invalid_argument if any argument is `null` or in an invalid state.
     * @throws std::runtime_error if an error occurs during creation of the benchmark.
     * @throws any exception that the underlying benchmark generation system throws.
     * @details Only Engine can create a benchmark, thus, this method is private,
     * accessible only by friend class Engine.
     */
    static IBenchmark::Ptr createBenchmark(std::shared_ptr<Engine> p_engine,
                                           IBenchmarkDescriptor::DescriptionToken::Ptr p_token,
                                           hebench::Utilities::TimingReportEx &out_report);

    BenchmarkFactory() = default;
};

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_Benchmark_Factory_H_0596d40a3cce4b108a81595c50eb286d
