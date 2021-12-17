
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_DataGenerator_DotProduct_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_DataGenerator_DotProduct_H_0596d40a3cce4b108a81595c50eb286d

#include <array>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "modules/general/include/nocopy.h"
#include "modules/logging/include/logging.h"

#include "hebench/api_bridge/types.h"

#include "include/hebench_benchmark_factory.h"
#include "include/hebench_idata_loader.h"

namespace hebench {
namespace TestHarness {
namespace DotProduct {

class BenchmarkDescriptorCategory : public hebench::TestHarness::PartialBenchmarkDescriptor
{
public:
    DISABLE_COPY(BenchmarkDescriptorCategory)
    DISABLE_MOVE(BenchmarkDescriptorCategory)
private:
    IL_DECLARE_CLASS_NAME(DotProduct::BenchmarkDescriptorCategory)
public:
    static constexpr const char *BaseWorkloadName         = "Dot Product";
    static constexpr std::uint64_t WorkloadParameterCount = 1; // number of parameters for this workload
    static constexpr std::uint64_t OpParameterCount       = 2; // number of parameters for this operation
    static constexpr std::uint64_t OpResultCount          = 1; // number of outputs for this operation
    static hebench::APIBridge::WorkloadParamType::WorkloadParamType WorkloadParameterType[WorkloadParameterCount];

    static std::uint64_t fetchVectorSize(const std::vector<hebench::APIBridge::WorkloadParam> &w_params);

public:
    BenchmarkDescriptorCategory()           = default;
    ~BenchmarkDescriptorCategory() override = default;

protected:
    bool matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                  const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const override;
    void completeWorkloadDescription(WorkloadDescriptionOutput &output,
                                     const Engine &engine,
                                     const BenchmarkDescription::Backend &backend_desc,
                                     const BenchmarkDescription::Configuration &config) const override;
};

class DataGenerator : public hebench::TestHarness::PartialDataLoader
{
public:
    DISABLE_COPY(DataGenerator)
    DISABLE_MOVE(DataGenerator)
private:
    IL_DECLARE_CLASS_NAME(DotProduct::DataGenerator)

public:
    typedef std::shared_ptr<DataGenerator> Ptr;

    static DataGenerator::Ptr create(std::uint64_t vector_size,
                                     std::uint64_t batch_size_a,
                                     std::uint64_t batch_size_b,
                                     hebench::APIBridge::DataType data_type);

    ~DataGenerator() override = default;

private:
    static constexpr std::size_t InputDim0  = BenchmarkDescriptorCategory::OpParameterCount;
    static constexpr std::size_t OutputDim0 = BenchmarkDescriptorCategory::OpResultCount;

    DataGenerator() = default;
    void init(std::uint64_t vector_size,
              std::uint64_t batch_size_a,
              std::uint64_t batch_size_b,
              hebench::APIBridge::DataType data_type);
};

} // namespace DotProduct
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_DataGenerator_DotProduct_H_0596d40a3cce4b108a81595c50eb286d
