
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_DataGenerator_MatrixMultiply_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_DataGenerator_MatrixMultiply_H_0596d40a3cce4b108a81595c50eb286d

#include <array>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "hebench/modules/general/include/nocopy.h"
#include "hebench/modules/logging/include/logging.h"

#include "hebench/api_bridge/types.h"

#include "benchmarks/datagen_helper/include/datagen_helper.h"
#include "include/hebench_benchmark_factory.h"

namespace hebench {
namespace TestHarness {
namespace MatrixMultiply {

class BenchmarkDescriptorCategory : public hebench::TestHarness::PartialBenchmarkDescriptor
{
public:
    DISABLE_COPY(BenchmarkDescriptorCategory)
    DISABLE_MOVE(BenchmarkDescriptorCategory)
private:
    IL_DECLARE_CLASS_NAME(MatrixMultiply::BenchmarkDescriptorCategory)
public:
    static constexpr const char *BaseWorkloadName         = "Matrix Multiplication";
    static constexpr std::uint64_t WorkloadParameterCount = 3; // number of parameters for this workload
    static constexpr std::uint64_t OpParameterCount       = 2; // number of parameters for this operation
    static constexpr std::uint64_t OpResultCount          = 1; // number of outputs for this operation
    static hebench::APIBridge::WorkloadParamType::WorkloadParamType WorkloadParameterType[WorkloadParameterCount];

    /**
     * @brief fetchMatrixSizes
     * @param w_params
     * @return Rows and columns for each input matrix. Each pair represents the
     * dimensions of a matrix as such: first <=> rows, seconds <=> cols.
     */
    static std::array<std::pair<std::uint64_t, std::uint64_t>, OpParameterCount>
    fetchMatrixSizes(const std::vector<hebench::APIBridge::WorkloadParam> &w_params);

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

class DataLoader : public hebench::TestHarness::DataLoaderCompute
{
public:
    DISABLE_COPY(DataLoader)
    DISABLE_MOVE(DataLoader)
private:
    IL_DECLARE_CLASS_NAME(MatrixMultiply::DataLoader)

public:
    typedef std::shared_ptr<DataLoader> Ptr;

    static DataLoader::Ptr create(std::uint64_t rows_a, std::uint64_t cols_a, std::uint64_t cols_b,
                                  std::uint64_t batch_size_mat_a,
                                  std::uint64_t batch_size_mat_b,
                                  hebench::APIBridge::DataType data_type);
    static DataLoader::Ptr create(std::uint64_t rows_a, std::uint64_t cols_a, std::uint64_t cols_b,
                                  std::uint64_t expected_sample_size_mat_a,
                                  std::uint64_t expected_sample_size_mat_b,
                                  hebench::APIBridge::DataType data_type,
                                  const std::string &dataset_filename);

    ~DataLoader() override {}

protected:
    void computeResult(std::vector<hebench::APIBridge::NativeDataBuffer *> &result,
                       const std::uint64_t *param_data_pack_indices,
                       hebench::APIBridge::DataType data_type) override;

private:
    static constexpr std::size_t InputDim0  = 2;
    static constexpr std::size_t OutputDim0 = 1;
    std::uint64_t m_rows_a;
    std::uint64_t m_cols_a;
    std::uint64_t m_cols_b;

    DataLoader();
    void init(std::uint64_t rows_a, std::uint64_t cols_a, std::uint64_t cols_b,
              std::uint64_t batch_size_mat_a,
              std::uint64_t batch_size_mat_b,
              hebench::APIBridge::DataType data_type);
    void init(std::uint64_t rows_a, std::uint64_t cols_a, std::uint64_t cols_b,
              std::uint64_t expected_sample_size_mat_a,
              std::uint64_t expected_sample_size_mat_b,
              hebench::APIBridge::DataType data_type,
              const std::string &dataset_filename);
};

} // namespace MatrixMultiply
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_DataGenerator_MatrixMultiply_H_0596d40a3cce4b108a81595c50eb286d
