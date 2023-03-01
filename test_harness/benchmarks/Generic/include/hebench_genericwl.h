
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_DataGenerator_GenericWL_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_DataGenerator_GenericWL_H_0596d40a3cce4b108a81595c50eb286d

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
namespace GenericWL {

class BenchmarkDescriptorCategory : public hebench::TestHarness::PartialBenchmarkDescriptor
{
public:
    DISABLE_COPY(BenchmarkDescriptorCategory)
    DISABLE_MOVE(BenchmarkDescriptorCategory)
private:
    IL_DECLARE_CLASS_NAME(GenericWL::BenchmarkDescriptorCategory)
public:
    static constexpr const char *BaseWorkloadName            = "Generic";
    static constexpr std::uint64_t WorkloadParameterMinCount = 4; // minimum number of parameters for this workload
    static constexpr hebench::APIBridge::WorkloadParamType::WorkloadParamType WorkloadParameterType =
        hebench::APIBridge::WorkloadParamType::UInt64;

    //static std::uint64_t fetchVectorSize(const std::vector<hebench::APIBridge::WorkloadParam> &w_params);
    /**
     * @brief Retrieves details about the input parameters and results of the
     * generic operation based on the specified flexible workload parameters.
     * @param[in] w_params Flexible workload parameters describing the operation.
     * @return A pair where the first element represents the vector sizes for
     * each component of the operation input; the second element represents the
     * vector sizes for each component of the operation output.
     */
    static std::pair<std::vector<std::uint64_t>, std::vector<std::uint64_t>> fetchIOVectorSizes(const std::vector<hebench::APIBridge::WorkloadParam> &w_params);

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
    IL_DECLARE_CLASS_NAME(GenericWL::DataLoader)

public:
    typedef std::shared_ptr<DataLoader> Ptr;

    static DataLoader::Ptr create(const std::vector<std::uint64_t> &input_sizes,
                                  const std::vector<std::uint64_t> &max_batch_sizes,
                                  const std::vector<std::uint64_t> &output_sizes,
                                  hebench::APIBridge::DataType data_type,
                                  const std::string &dataset_filename);

    ~DataLoader() override {}

protected:
    void computeResult(std::vector<hebench::APIBridge::NativeDataBuffer *> &result,
                       const std::uint64_t *param_data_pack_indices,
                       hebench::APIBridge::DataType data_type) override;

private:
    DataLoader();
    void init(const std::vector<std::uint64_t> &input_sizes,
              const std::vector<std::uint64_t> &max_batch_sizes,
              const std::vector<std::uint64_t> &output_sizes,
              hebench::APIBridge::DataType data_type,
              const std::string &dataset_filename);
};

} // namespace GenericWL
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_DataGenerator_GenericWL_H_0596d40a3cce4b108a81595c50eb286d
