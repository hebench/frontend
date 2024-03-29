
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_GenericWL_O_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_GenericWL_O_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>

#include "hebench/modules/general/include/nocopy.h"
#include "hebench/modules/logging/include/logging.h"

#include "benchmarks/Generic/include/hebench_genericwl.h"
#include "benchmarks/categories/include/hebench_benchmark_offline.h"
#include "hebench/api_bridge/types.h"

namespace hebench {
namespace TestHarness {
namespace GenericWL {
namespace Offline {

class BenchmarkDescriptor final : public hebench::TestHarness::GenericWL::BenchmarkDescriptorCategory
{
public:
    DISABLE_COPY(BenchmarkDescriptor)
    DISABLE_MOVE(BenchmarkDescriptor)
private:
    IL_DECLARE_CLASS_NAME(GenericWL::Offline::BenchmarkDescriptor)
public:
    static constexpr std::uint32_t BenchmarkID = 1101;
    // default number of samples for an operand
    // (based on docs definition of the workload operation)
    static constexpr std::uint64_t DefaultBatchSize = 1;

public:
    BenchmarkDescriptor()           = default;
    ~BenchmarkDescriptor() override = default;

    hebench::TestHarness::PartialBenchmark *createBenchmark(std::shared_ptr<Engine> p_engine,
                                                            const DescriptionToken &description_token) override;
    void destroyBenchmark(hebench::TestHarness::PartialBenchmark *p_bench) override;

protected:
    bool matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                  const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const override;
    void completeWorkloadDescription(WorkloadDescriptionOutput &output,
                                     const Engine &engine,
                                     const BenchmarkDescription::Backend &backend_desc,
                                     const BenchmarkDescription::Configuration &config) const override;

private:
    static bool m_b_registered;
};

class Benchmark : public hebench::TestHarness::BenchmarkOffline
{
public:
    DISABLE_COPY(Benchmark)
    DISABLE_MOVE(Benchmark)
private:
    IL_DECLARE_CLASS_NAME(GenericWL::Offline::Benchmark)

public:
    friend class hebench::TestHarness::GenericWL::Offline::BenchmarkDescriptor;

    ~Benchmark() override = default;

protected:
    void init() override;
    IDataLoader::Ptr getDataset() const override { return m_data; }
    std::uint32_t getEventIDStart() const override { return BenchmarkDescriptor::BenchmarkID; }
    bool validateResult(IDataLoader::Ptr dataset,
                        const std::uint64_t *param_data_pack_indices,
                        const std::vector<hebench::APIBridge::NativeDataBuffer *> &p_outputs,
                        hebench::APIBridge::DataType data_type) const override;

private:
    std::uint64_t m_op_input_count;
    std::uint64_t m_op_output_count;
    DataLoader::Ptr m_data;

    Benchmark(std::shared_ptr<Engine> p_engine,
              const IBenchmarkDescriptor::DescriptionToken &description_token);
};

} // namespace Offline
} // namespace GenericWL
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_GenericWL_O_H_0596d40a3cce4b108a81595c50eb286d
