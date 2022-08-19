
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_DataGenHelper_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_DataGenHelper_H_0596d40a3cce4b108a81595c50eb286d

#include <mutex>
#include <string>
#include <set>
#include <type_traits>

#include "modules/logging/include/logging.h"

#include "hebench/api_bridge/types.h"
#include "include/hebench_idata_loader.h"

namespace hebench {
namespace TestHarness {

class DataLoaderCompute : public hebench::TestHarness::PartialDataLoader
{
public:
    DISABLE_COPY(DataLoaderCompute)
    DISABLE_MOVE(DataLoaderCompute)
private:
    IL_DECLARE_CLASS_NAME(DataLoaderCompute)
public:
    ~DataLoaderCompute() override {}
    ResultDataPtr getResultFor(const std::uint64_t *param_data_pack_indices) override;

protected:
    DataLoaderCompute() {}

    /**
     * @brief Computes result of the operation on the input data given the
     * of the input sample.
     * @param result Vector where to store the result. Vector comes pre-initialized.
     * This method's task is to fill up the data buffers with the result values.
     * @param[in] param_data_pack_indices For each operation parameter, this array
     * indicates the sample index to use for the operation.
     * Must contain, at least, `getParameterCount()` elements.
     * @param[in] data_type Data type of the data pointed by the inputs and result.
     */
    virtual void computeResult(std::vector<hebench::APIBridge::NativeDataBuffer *> &result,
                               const std::uint64_t *param_data_pack_indices,
                               hebench::APIBridge::DataType data_type) = 0;

private:
    // can we find a thread friendly/safe solution?
    std::vector<std::shared_ptr<hebench::APIBridge::DataPack>> &getLocalTempResult();
    std::shared_ptr<std::vector<std::shared_ptr<hebench::APIBridge::DataPack>>> m_p_temp_result;
};

/**
 * @brief Static helper class to generate vector data for all supported data types.
 * @details This class uses the default seed for generation to ensure repeatable results
 * as long as parameters for the tests are the same.
 */
class DataGeneratorHelper
{
private:
    IL_DECLARE_CLASS_NAME(DataGeneratorHelper)

public:
    virtual ~DataGeneratorHelper() = default;

    /**
     * @brief Generates uniform random data of the specified type.
     * @param[in] data_type Data type of data to generate.
     * @param[in] result Pointer to buffer containing elements of the specified type.
     * @param[in] elem_count Number of elements in pointed buffer.
     * @param[in] min_val Minimum value for generated elements.
     * @param[in] max_val Maximum value for generated elements.
     */
    static void generateRandomVectorU(hebench::APIBridge::DataType data_type,
                                      void *result, std::uint64_t elem_count,
                                      double min_val, double max_val);
    /**
     * @brief Generates normally distributed random data of the specified type.
     * @param[in] data_type Data type of data to generate.
     * @param[in] result Pointer to buffer containing elements of the specified type.
     * @param[in] elem_count Number of elements in pointed buffer.
     * @param[in] min_val Minimum value for generated elements.
     * @param[in] max_val Maximum value for generated elements.
     */
    static void generateRandomVectorN(hebench::APIBridge::DataType data_type,
                                      void *result, std::uint64_t elem_count,
                                      double mean, double stddev);

    /**
     * @brief Generates normally distributed random data of the specified type.
     * @param[in] data_type Data type of data to generate.
     * @param[in] result Pointer to buffer containing elements of the specified type.
     * @param[in] elem_count Number of elements in pointed buffer.
     * @param[in] min_val Minimum value for generated elements.
     * @param[in] max_val Maximum value for generated elements.
     */
    static void generateRandomSetN(hebench::APIBridge::DataType data_type,
                                      void *result, std::uint64_t elem_count,
                                      double mean, double stddev);

protected:
    DataGeneratorHelper() = default;

private:
    template <class T>
    static void generateRandomVectorU(T *result, std::uint64_t elem_count,
                                      T min_val, T max_val);
    template <class T>
    static void generateRandomVectorN(T *result, std::uint64_t elem_count,
                                      T mean, T stddev);
    template <class T>
    static void generateRandomSetN(T *result, std::uint64_t elem_count,
                                   T mean, T stddev);
    static void generateRandomStringSetN(std::string *result, std::uint64_t elem_count,
                                         double mean, double stddev);

private:
    static std::mutex m_mtx_rand;
};

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_DataGenerator_H_0596d40a3cce4b108a81595c50eb286d
