
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <chrono>
#include <stdexcept>

#include "../include/datagen_helper.h"

namespace hebench {
namespace TestHarness {

std::mutex DataGeneratorHelper::m_mtx_rand;

void DataGeneratorHelper::generateRandomVectorU(hebench::APIBridge::DataType data_type,
                                                void *result, std::uint64_t elem_count,
                                                double min_val, double max_val)
{
    switch (data_type)
    {
    case hebench::APIBridge::DataType::Int32:
        generateRandomVectorU<std::int32_t>(reinterpret_cast<std::int32_t *>(result), elem_count,
                                            static_cast<std::int32_t>(min_val), static_cast<std::int32_t>(max_val));
        break;

    case hebench::APIBridge::DataType::Int64:
        generateRandomVectorU<std::int64_t>(reinterpret_cast<std::int64_t *>(result), elem_count,
                                            static_cast<std::int64_t>(min_val), static_cast<std::int64_t>(max_val));
        break;

    case hebench::APIBridge::DataType::Float32:
        generateRandomVectorU<float>(reinterpret_cast<float *>(result), elem_count,
                                     static_cast<float>(min_val), static_cast<float>(max_val));
        break;

    case hebench::APIBridge::DataType::Float64:
        generateRandomVectorU<double>(reinterpret_cast<double *>(result), elem_count,
                                      static_cast<double>(min_val), static_cast<double>(max_val));
        break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Unknown data type."));
        break;
    } // end switch
}

void DataGeneratorHelper::generateRandomVectorN(hebench::APIBridge::DataType data_type,
                                                void *result, std::uint64_t elem_count,
                                                double mean, double stddev)
{
    switch (data_type)
    {
    case hebench::APIBridge::DataType::Int32:
        generateRandomVectorN<std::int32_t>(reinterpret_cast<std::int32_t *>(result), elem_count,
                                            static_cast<std::int32_t>(mean), static_cast<std::int32_t>(stddev));
        break;

    case hebench::APIBridge::DataType::Int64:
        generateRandomVectorN<std::int64_t>(reinterpret_cast<std::int64_t *>(result), elem_count,
                                            static_cast<std::int64_t>(mean), static_cast<std::int64_t>(stddev));
        break;

    case hebench::APIBridge::DataType::Float32:
        generateRandomVectorN<float>(reinterpret_cast<float *>(result), elem_count,
                                     static_cast<float>(mean), static_cast<float>(stddev));
        break;

    case hebench::APIBridge::DataType::Float64:
        generateRandomVectorN<double>(reinterpret_cast<double *>(result), elem_count,
                                      static_cast<double>(mean), static_cast<double>(stddev));
        break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Unknown data type."));
        break;
    } // end switch
}

} // namespace TestHarness
} // namespace hebench
