
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_DataGenerator_LogisticRegression_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_DataGenerator_LogisticRegression_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>

#include "modules/general/include/nocopy.h"
#include "modules/logging/include/logging.h"

#include "hebench/api_bridge/types.h"

#include "include/hebench_benchmark_factory.h"
#include "include/hebench_idata_loader.h"

namespace hebench {
namespace TestHarness {
namespace LogisticRegression {

class BenchmarkDescriptorCategory : public hebench::TestHarness::PartialBenchmarkDescriptor
{
public:
    DISABLE_COPY(BenchmarkDescriptorCategory)
    DISABLE_MOVE(BenchmarkDescriptorCategory)
private:
    IL_DECLARE_CLASS_NAME(LogisticRegression::BenchmarkDescriptorCategory)
public:
    static constexpr const char *BaseWorkloadName         = "Logistic Regression";
    static constexpr std::uint64_t WorkloadParameterCount = 1; // number of parameters for this workload
    static constexpr std::uint64_t OpParameterCount       = 3; // number of parameters for this operation
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
    IL_DECLARE_CLASS_NAME(LogisticRegression::DataGenerator)

public:
    typedef std::shared_ptr<DataGenerator> Ptr;
    enum class PolynomialDegree
    {
        None,
        PD3,
        PD5,
        PD7
    };

    static constexpr std::size_t Index_W = 0;
    static constexpr std::size_t Index_b = 1;
    static constexpr std::size_t Index_X = 2;

    static DataGenerator::Ptr create(PolynomialDegree polynomial_degree,
                                     std::uint64_t vector_size,
                                     std::uint64_t batch_size_input,
                                     hebench::APIBridge::DataType data_type);

    ~DataGenerator() override = default;

private:
    static constexpr std::size_t InputDim0  = BenchmarkDescriptorCategory::OpParameterCount;
    static constexpr std::size_t OutputDim0 = BenchmarkDescriptorCategory::OpResultCount;

    DataGenerator() = default;
    void init(PolynomialDegree polynomial_degree,
              std::uint64_t vector_size,
              std::uint64_t batch_size_input,
              hebench::APIBridge::DataType data_type);
};

} // namespace LogisticRegression
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_DataGenerator_LogisticRegression_H_0596d40a3cce4b108a81595c50eb286d
