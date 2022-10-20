
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>

#include "../include/datagen_helper.h"
#include "include/hebench_utilities_harness.h"

namespace hebench {
namespace TestHarness {

std::mutex DataGeneratorHelper::m_mtx_rand;

//-------------------------
// class DataLoaderCompute
//-------------------------

IDataLoader::ResultDataPtr DataLoaderCompute::getResultFor(const std::uint64_t *param_data_pack_indices)
{
    if (!this->isInitialized())
        throw std::logic_error(IL_LOG_MSG_CLASS("Not initialized."));

    ResultDataPtr p_retval;

    if (this->hasResults())
    {
        p_retval = PartialDataLoader::getResultFor(param_data_pack_indices);
    } // end if
    else
    {
        std::uint64_t r_i = getResultIndex(param_data_pack_indices);
        std::shared_ptr<std::vector<std::shared_ptr<hebench::APIBridge::DataPack>>> p_tmp_packs =
            std::make_shared<std::vector<std::shared_ptr<hebench::APIBridge::DataPack>>>(getResultTempDataPacks());
        std::vector<std::shared_ptr<hebench::APIBridge::DataPack>> &tmp_packs = *p_tmp_packs;
        // point to the `NativeDataBuffer`s to contain the result
        std::vector<hebench::APIBridge::NativeDataBuffer *> result(tmp_packs.size());
        for (std::size_t result_component_i = 0; result_component_i < result.size(); ++result_component_i)
        {
            assert(tmp_packs[result_component_i]
                   && tmp_packs[result_component_i]->buffer_count > 0
                   && tmp_packs[result_component_i]->p_buffers
                   && tmp_packs[result_component_i]->p_buffers[0].p
                   && tmp_packs[result_component_i]->p_buffers[0].size > 0);
            result[result_component_i] = &(tmp_packs[result_component_i]->p_buffers[0]);
        } // end for
        // compute result and store in the pre-allocated buffers
        computeResult(result, param_data_pack_indices, getDataType());

        p_retval = ResultDataPtr(new ResultData());
        p_retval->result.assign(result.begin(), result.end()); // pointers to pre-allocated data
        p_retval->sample_index = r_i;
        p_retval->reserved0    = p_tmp_packs; // pre-allocated data as a RAII
    } // end if

    return p_retval;
}

//---------------------------
// class DataGeneratorHelper
//---------------------------

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

std::uint64_t DataGeneratorHelper::generateRandomIntU(std::uint64_t min_val, std::uint64_t max_val)
{
    std::uniform_int_distribution<std::uint64_t> rnd(min_val, max_val);
    return rnd(hebench::Utilities::RandomGenerator::get());
}

std::vector<std::uint64_t> DataGeneratorHelper::generateRandomIntersectionIndicesU(std::uint64_t elem_count, std::uint64_t indices_count)
{
    std::vector<std::uint64_t> retval;

    if (indices_count <= 0)
        indices_count = generateRandomIntU(0, elem_count - 1);

    retval.resize(elem_count);
    std::iota(retval.begin(), retval.end(), 0);
    std::shuffle(retval.begin(), retval.end(), hebench::Utilities::RandomGenerator::get());

    return std::vector<std::uint64_t>(retval.begin(), retval.begin() + indices_count);
}

} // namespace TestHarness
} // namespace hebench
