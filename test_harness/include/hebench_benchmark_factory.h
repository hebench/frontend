
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
    /**
     * @brief Contains an internal representation of a matched benchmark.
     * @details This token is returned by matchBenchmarkDescriptor(), representing a
     * successful match. Returned tokens can be used to instantiate the actual
     * benchmarks they represent.
     */
    class BenchmarkToken
    {
    private:
        BenchmarkToken(std::shared_ptr<IBenchmarkDescription> p_bmd,
                       IBenchmarkDescription::DescriptionToken::Ptr p_bdt) :
            m_p_bmd(p_bmd),
            m_p_bdt(p_bdt), description(m_p_bdt->description)
        {
        }

        std::shared_ptr<IBenchmarkDescription> m_p_bmd;
        IBenchmarkDescription::DescriptionToken::Ptr m_p_bdt;

    public:
        friend class BenchmarkFactory;
        typedef std::shared_ptr<BenchmarkToken> Ptr;

        /**
         * @brief Description of the benchmark matched.
         * @details This can be used by caller to obtain description information about the
         * matched benchmark.
         */
        const IBenchmarkDescription::Description &description;
    };

    friend class Engine;

    /**
     * @brief Returns a token representing a benchmark that can perform the described
     * workload with specified parameters, if any.
     * @param[in] engine Calling engine object.
     * @param[in] h_desc HEBench handle to descriptor of the benchmark to perform.
     * @param[in] w_params Parameters for the benchmark's workload.
     * @return The token representing the matched benchmark, or `null` if no match is found.
     */
    static BenchmarkToken::Ptr matchBenchmarkDescriptor(const Engine &engine,
                                                        const IBenchmarkDescription::BenchmarkConfig &bench_config,
                                                        const hebench::APIBridge::Handle &h_desc,
                                                        const std::vector<hebench::APIBridge::WorkloadParam> &w_params);
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
    static bool registerSupportedBenchmark(std::shared_ptr<IBenchmarkDescription> p_desc_obj);

private:
    static std::vector<std::shared_ptr<IBenchmarkDescription>> &getRegisteredBenchmarks();
    static BenchmarkToken::Ptr createBenchmarkToken(std::shared_ptr<IBenchmarkDescription> p_bmd,
                                                    IBenchmarkDescription::DescriptionToken::Ptr p_bdt);
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
                                           BenchmarkToken::Ptr p_token,
                                           hebench::Utilities::TimingReportEx &out_report);

    BenchmarkFactory() = default;
};

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_Benchmark_Factory_H_0596d40a3cce4b108a81595c50eb286d
