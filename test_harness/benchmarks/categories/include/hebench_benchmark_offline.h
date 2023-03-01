
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Benchmark_Offline_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Benchmark_Offline_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>
#include <string>
#include <vector>

#include "hebench/modules/general/include/nocopy.h"
#include "hebench/modules/logging/include/logging.h"

#include "hebench_benchmark_category.h"

namespace hebench {
namespace TestHarness {

class Engine;

/**
 * @brief Base class for workload benchmarks in the offline category.
 * @details This class offers an implementation to offline category benchmarking.
 *
 * To extend this class, first clients must understand the execution flow as specified
 * in IBenchmark interface. Then, see \ref extend_test_harness_o .
 */
class BenchmarkOffline : public PartialBenchmarkCategory
{
public:
    DISABLE_COPY(BenchmarkOffline)
    DISABLE_MOVE(BenchmarkOffline)
private:
    IL_DECLARE_CLASS_NAME(BenchmarkOffline)

public:
    typedef std::shared_ptr<BenchmarkOffline> Ptr;

    ~BenchmarkOffline() override;

    /**
     * @brief Executes the benchmark offline test.
     * @param out_report Object where to append the report of the operation.
     * @param config Specifies configuration parameters for the run.
     * @returns true if benchmark succeeded and operation results were valid.
     * @returns false if benchmark operation results were not valid.
     * @details
     * In this implementation, validation failure is reported on the first
     * result that fails validation.
     *
     * - Returns when no errors (`true`), or validation failed (`false`).
     * - hebench::Common::ErrorException with backend error number attached if
     * backend errors occur.
     * - Throws std::exception (or derived exception other than hebench::Common::ErrorException)
     * on any other errors.
     */
    bool run(hebench::Utilities::TimingReportEx &out_report, IBenchmark::RunConfig &config) override;

protected:
    BenchmarkOffline(std::shared_ptr<Engine> p_engine,
                     const IBenchmarkDescriptor::DescriptionToken &description_token);

private:
    bool run(hebench::Utilities::TimingReportEx &out_report,
             IDataLoader::Ptr p_dataset,
             IBenchmark::RunConfig &run_config);
};

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_Benchmark_Offline_H_0596d40a3cce4b108a81595c50eb286d
