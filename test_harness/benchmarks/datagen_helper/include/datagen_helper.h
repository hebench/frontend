
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_DataGenHelper_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_DataGenHelper_H_0596d40a3cce4b108a81595c50eb286d

#include <mutex>

#include "modules/logging/include/logging.h"

#include "hebench/api_bridge/types.h"

namespace hebench {
namespace TestHarness {

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

protected:
    DataGeneratorHelper() = default;

private:
    template <class T>
    static void generateRandomVectorU(T *result, std::uint64_t elem_count,
                                      T min_val, T max_val);
    template <class T>
    static void generateRandomVectorN(T *result, std::uint64_t elem_count,
                                      T mean, T stddev);

private:
    static std::mutex m_mtx_rand;
};

} // namespace TestHarness
} // namespace hebench

#include "inl/datagen_helper.inl"

#endif // defined _HEBench_Harness_DataGenerator_H_0596d40a3cce4b108a81595c50eb286d
