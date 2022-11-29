
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_DataGenerator_DotProduct_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_DataGenerator_DotProduct_H_0596d40a3cce4b108a81595c50eb286d

#include <algorithm>
#include <array>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "modules/general/include/hebench_math_utils.h"
#include "modules/general/include/nocopy.h"
#include "modules/logging/include/logging.h"

#include "hebench/api_bridge/types.h"

#include "benchmarks/datagen_helper/include/datagen_helper.h"
#include "include/hebench_benchmark_factory.h"

namespace hebench {
namespace TestHarness {
namespace SimpleSetIntersection {

class BenchmarkDescriptorCategory : public hebench::TestHarness::PartialBenchmarkDescriptor
{
public:
    DISABLE_COPY(BenchmarkDescriptorCategory)
    DISABLE_MOVE(BenchmarkDescriptorCategory)
private:
    IL_DECLARE_CLASS_NAME(DotProduct::BenchmarkDescriptorCategory)
public:
    static constexpr const char *BaseWorkloadName         = "Simple Set Intersection";
    static constexpr std::uint64_t WorkloadParameterCount = 5; // number of parameters for this workload
    static constexpr std::uint64_t OpParameterCount       = 2; // number of parameters for this operation
    static constexpr std::uint64_t OpResultCount          = 1; // number of outputs for this operation
    static hebench::APIBridge::WorkloadParamType::WorkloadParamType WorkloadParameterType[WorkloadParameterCount];

    static std::array<std::int64_t, BenchmarkDescriptorCategory::WorkloadParameterCount> fetchSetSize(const std::vector<hebench::APIBridge::WorkloadParam> &w_params);

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
    IL_DECLARE_CLASS_NAME(DotProduct::DataLoader)

public:
    typedef std::shared_ptr<DataLoader> Ptr;

    static DataLoader::Ptr create(std::uint64_t set_size_x,
                                  std::uint64_t set_size_y,
                                  std::uint64_t batch_size_x,
                                  std::uint64_t batch_size_y,
                                  std::uint64_t element_size_k,
                                  std::int64_t data_range_i,
                                  std::int64_t data_range_j,
                                  hebench::APIBridge::DataType data_type);
    static DataLoader::Ptr create(std::uint64_t set_size_x,
                                  std::uint64_t set_size_y,
                                  std::uint64_t batch_size_x,
                                  std::uint64_t batch_size_y,
                                  std::uint64_t element_size_k,
                                  std::int64_t data_range_i,
                                  std::int64_t data_range_j,
                                  hebench::APIBridge::DataType data_type,
                                  const std::string &dataset_filename);

    ~DataLoader() override {}

protected:
    void computeResult(std::vector<hebench::APIBridge::NativeDataBuffer *> &result,
                       const std::uint64_t *param_data_pack_indices,
                       hebench::APIBridge::DataType data_type) override;

private:
    static constexpr std::size_t InputDim0  = BenchmarkDescriptorCategory::OpParameterCount;
    static constexpr std::size_t OutputDim0 = BenchmarkDescriptorCategory::OpResultCount;
    std::uint64_t m_set_size_x;
    std::uint64_t m_set_size_y;
    std::uint64_t m_element_size_k;
    std::int64_t m_data_range_i;
    std::int64_t m_data_range_j;

    DataLoader();
    void init(std::uint64_t set_size_x,
              std::uint64_t set_size_y,
              std::uint64_t batch_size_x,
              std::uint64_t batch_size_y,
              std::uint64_t element_size_k,
              std::int64_t data_range_i,
              std::int64_t data_range_j,
              hebench::APIBridge::DataType data_type);
    void init(std::uint64_t set_size_x,
              std::uint64_t set_size_y,
              std::uint64_t batch_size_x,
              std::uint64_t batch_size_y,
              std::uint64_t element_size_k,
              std::int64_t data_range_i,
              std::int64_t data_range_j,
              hebench::APIBridge::DataType data_type,
              const std::string &dataset_filename);
};

template <typename T>
/**
* @brief Finds whether values in two arrays are within a certain percentage of each other.
* @param[in] a Pointer to start of first array to compare.
* @param[in] b Pointer to start of second array to compare.
* @param[in] element_count Number of elements in array \p a.
* @param[in] pct Per-one for comparison: this is percent divided by 100.
* @return A vector of `uint64` where each element in this vector is the index of the
* values in \p a and \p b that were not within \p pct * 100 of each other. The return
* vector is empty if all values were within range of each other.
* @details Parameter \p a must hold, at least, \p count elements.
*/
typename std::enable_if<std::is_integral<T>::value
                            || std::is_floating_point<T>::value,
                        std::vector<std::pair<bool, std::uint64_t>>>::type
almostEqualSet(const T *X, const T *Y,
               std::uint64_t n, std::uint64_t m, std::uint64_t k,
               double pct = 0.05);

template <typename T>
/**
* @brief Finds whether values in two arrays are within a certain percentage of each other.
* @param[in] a Pointer to start of the array.
* @param[in] element value to be found in a.
* @param[in] element_count Number of elements in array \p a.
* @param[in] item_count Number of items per element in array \p a.
* @param[in] pct Per-one for comparison: this is percent divided by 100.
* @return Returns a `boolean` `true` if \p element is a member of \p a, `false` otherwise.
* @details Parameter \p a must hold, at least, \p count elements.
*/
typename std::enable_if<std::is_integral<T>::value
                            || std::is_floating_point<T>::value,
                        bool>::type
isMemberOf(const T *dataset, const T *value, std::size_t n, std::size_t k,
           double pct = 0.05);

bool validateResult(IDataLoader::Ptr dataset,
                    const std::uint64_t *param_data_pack_indices,
                    const std::vector<hebench::APIBridge::NativeDataBuffer *> &p_outputs,
                    std::uint64_t k_count,
                    hebench::APIBridge::DataType data_type);

// Simple Set Intersection inline impl.

template <typename T>
typename std::enable_if<std::is_integral<T>::value
                            || std::is_floating_point<T>::value,
                        bool>::type
isMemberOf(const T *dataset, const T *value, std::size_t n, std::size_t k,
           double pct)
{
    bool retval = false;
    for (size_t i = 0; !retval && i < n; ++i)
    {
        std::uint64_t members = 0;
        bool flag             = true;
        for (size_t j = 0; flag && j < k; ++j)
        {
            flag = hebench::Utilities::Math::almostEqual(dataset[(i * k) + j], value[j], pct);
            if (flag)
            {
                ++members;
            }
        }
        retval = members == k;
    }
    return retval;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value
                            || std::is_floating_point<T>::value,
                        std::vector<std::pair<bool, std::uint64_t>>>::type
almostEqualSet(const T *X, const T *Y,
               std::uint64_t n, std::uint64_t m, std::uint64_t k,
               double pct)
{
    std::vector<std::pair<bool, std::uint64_t>> retval;
    retval.reserve(n + m);
    if (X != Y)
    {
        if (!X)
        {
            // X has no elements
            for (std::uint64_t i = 0; i < m; ++i)
                retval.push_back(std::pair(false, i));
        } // end if
        else if (!Y)
        {
            // Y has no elements
            for (std::uint64_t i = 0; i < n; ++i)
                retval.push_back(std::pair(true, i));
        } // end else if
        else
        {
            // check if X c Y
            for (std::uint64_t i = 0; i < n; ++i)
            {
                if (!isMemberOf(Y, X + (i * k), m, k, pct))
                    retval.push_back(std::pair(true, i));
            } // end for
            // check if Y c X
            for (std::uint64_t i = 0; i < m; ++i)
            {
                if (!isMemberOf(X, Y + (i * k), n, k, pct))
                    retval.push_back(std::pair(false, i));
            } // end for
            // if retval.empty(), then X == Y
        } // end else
    } // end if
    return retval;
}

} // namespace SimpleSetIntersection
} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_DataGenerator_DotProduct_H_0596d40a3cce4b108a81595c50eb286d
