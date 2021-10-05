
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Benchmark_Latency_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Benchmark_Latency_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>
#include <string>
#include <vector>

#include "modules/general/include/nocopy.h"
#include "modules/logging/include/logging.h"

#include "hebench_benchmark_category.h"

namespace hebench {
namespace TestHarness {

class Engine;

/**
 * @brief Base class for workload benchmarks in the latency category.
 * @details This class offers an implementation to latency category benchmarking.
 *
 * To extend this class, first clients must understand the execution flow as specified
 * in IBenchmark interface. Then, see \ref extend_test_harness_l .
 */
class BenchmarkLatency : public PartialBenchmarkCategory
{
public:
    DISABLE_COPY(BenchmarkLatency)
    DISABLE_MOVE(BenchmarkLatency)
private:
    IL_DECLARE_CLASS_NAME(BenchmarkLatency)

public:
    typedef std::shared_ptr<BenchmarkLatency> Ptr;

    ~BenchmarkLatency() override;

    /**
     * @brief Executes the benchmark latency test.
     * @param out_report Object where to append the report of the operation.
     * @param config Specifies configuration parameters for the run.
     * @returns true if benchmark succeeded and operation results were valid.
     * @returns false if benchmark operation results were not valid.
     * @details
     * In this implementation, validation failure is reported on the first
     * result that fails validation.
     *
     * - Returns when no errors (`true`), or validation failed (`false`).
     * - hebench::Common::ErrorException with back-end error number attached if
     * back-end errors occur.
     * - Throws std::exception (or derived exception other than hebench::Common::ErrorException)
     * on any other errors.
     */
    bool run(hebench::Utilities::TimingReportEx &out_report, IBenchmark::RunConfig &config) override;

protected:
    BenchmarkLatency(std::shared_ptr<Engine> p_engine,
                     const IBenchmarkDescription::DescriptionToken &description_token);

private:
    bool run(hebench::Utilities::TimingReportEx &out_report,
             IDataLoader::Ptr p_dataset,
             const IBenchmarkDescription::BenchmarkConfig bench_config,
             IBenchmark::RunConfig &run_config);
};

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_Benchmark_Latency_H_0596d40a3cce4b108a81595c50eb286d
