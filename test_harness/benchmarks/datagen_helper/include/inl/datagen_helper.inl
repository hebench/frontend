
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_DataGenerator_SRC_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_DataGenerator_SRC_0596d40a3cce4b108a81595c50eb286d

#include "../datagen_helper.h"
#include "include/hebench_utilities.h"
#include <algorithm>

namespace hebench {
namespace TestHarness {

template <class T>
inline void DataGeneratorHelper::generateRandomVectorU(T *result, std::uint64_t elem_count,
                                                       T min_val, T max_val)
{
    std::uniform_real_distribution<double> rnd(min_val, max_val);
    for (std::uint64_t i = 0; i < elem_count; ++i)
    {
        std::lock_guard<std::mutex> lock(m_mtx_rand);
        result[i] = static_cast<T>(rnd(hebench::Utilities::RandomGenerator::get()));
    } // end for
}

template <class T>
inline void DataGeneratorHelper::generateRandomVectorN(T *result, std::uint64_t elem_count,
                                                       T mean, T stddev)
{
    std::normal_distribution<double> rnd(mean, stddev);
    for (std::uint64_t i = 0; i < elem_count; ++i)
    {
        std::lock_guard<std::mutex> lock(m_mtx_rand);
        result[i] = static_cast<T>(rnd(hebench::Utilities::RandomGenerator::get()));
    } // end for
}

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_DataGenerator_H_0596d40a3cce4b108a81595c50eb286d
