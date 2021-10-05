
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_DotProduct_L_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_DotProduct_L_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>

#include "modules/general/include/nocopy.h"
#include "modules/logging/include/logging.h"

#include "benchmarks/DotProduct/include/hebench_dotproduct.h"
#include "benchmarks/categories/include/hebench_benchmark_latency.h"
#include "hebench/api_bridge/types.h"

namespace hebench {
namespace TestHarness {
namespace DotProduct {
namespace Latency {

class BenchmarkDescription final : public hebench::TestHarness::DotProduct::BenchmarkDescriptionCategory
{
public:
    DISABLE_COPY(BenchmarkDescription)
    DISABLE_MOVE(BenchmarkDescription)
private:
    IL_DECLARE_CLASS_NAME(DotProduct::Latency::BenchmarkDescription)
public:
    static constexpr std::uint32_t BenchmarkID      = 601;
    static constexpr std::uint64_t DefaultBatchSize = 1; // batch size for a parameter (always 1 for latency)

public:
    BenchmarkDescription()           = default;
    ~BenchmarkDescription() override = default;

    hebench::TestHarness::PartialBenchmark *createBenchmark(std::shared_ptr<Engine> p_engine,
                                                            DescriptionToken::Ptr p_description_token) override;
    void destroyBenchmark(hebench::TestHarness::PartialBenchmark *p_bench) override;

protected:
    std::string matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                         const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const override;
    void completeDescription(const Engine &engine,
                             DescriptionToken::Ptr pre_token) const override;

private:
    static bool m_b_registered;
};

class Benchmark final : public hebench::TestHarness::BenchmarkLatency
{
public:
    DISABLE_COPY(Benchmark)
    DISABLE_MOVE(Benchmark)
private:
    IL_DECLARE_CLASS_NAME(DotProduct::Latency::Benchmark)

public:
    friend class hebench::TestHarness::DotProduct::Latency::BenchmarkDescription;

    ~Benchmark() override = default;

protected:
    void init(const IBenchmarkDescription::Description &description) override;
    IDataLoader::Ptr getDataset() const override { return m_data; }
    std::uint32_t getEventIDStart() const override { return BenchmarkDescription::BenchmarkID; }
    bool validateResult(IDataLoader::Ptr dataset,
                        const std::uint64_t *param_data_pack_indices,
                        const std::vector<hebench::APIBridge::NativeDataBuffer *> &p_outputs,
                        hebench::APIBridge::DataType data_type) const override;

private:
    DataGenerator::Ptr m_data;

    Benchmark(std::shared_ptr<Engine> p_engine,
              const IBenchmarkDescription::DescriptionToken &description_token);
};

} // namespace Latency
} // namespace DotProduct
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_DotProduct_L_H_0596d40a3cce4b108a81595c50eb286d
