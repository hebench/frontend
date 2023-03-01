
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <iostream>
#include <numeric>
#include <sstream>
#include <utility>

#include "../include/hebench_simple_set_intersection.h"
#include "hebench/modules/general/include/hebench_math_utils.h"

namespace hebench {
namespace TestHarness {
namespace SimpleSetIntersection {

//------------------------------------
// class BenchmarkDescriptionCategory
//------------------------------------

hebench::APIBridge::WorkloadParamType::WorkloadParamType
    BenchmarkDescriptorCategory::WorkloadParameterType[BenchmarkDescriptorCategory::WorkloadParameterCount] = {
        hebench::APIBridge::WorkloadParamType::UInt64,
        hebench::APIBridge::WorkloadParamType::UInt64,
        hebench::APIBridge::WorkloadParamType::UInt64
    };

std::array<std::uint64_t, BenchmarkDescriptorCategory::WorkloadParameterCount> BenchmarkDescriptorCategory::fetchSetSize(const std::vector<hebench::APIBridge::WorkloadParam> &w_params)
{
    assert(WorkloadParameterCount == 3);
    assert(OpParameterCount == 2);
    assert(OpResultCount == 1);

    std::array<std::uint64_t, WorkloadParameterCount> retval;

    if (w_params.size() < WorkloadParameterCount)
    {
        std::stringstream ss;
        ss << "Insufficient workload parameters in 'w_params'. Expected " << WorkloadParameterCount
           << ", but " << w_params.size() << "received.";
        throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
    } // end if

    // validate parameters
    for (std::size_t i = 0; i < WorkloadParameterCount; ++i)
        if (w_params[i].data_type != WorkloadParameterType[i])
        {
            std::stringstream ss;
            ss << "Invalid type for workload parameter " << i
               << ". Expected type ID " << WorkloadParameterType[i] << ", but " << w_params[i].data_type << " received.";
            throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
        } // end if
        else if (w_params[i].u_param <= 0)
        {
            std::stringstream ss;
            ss << "Invalid number of elements for vector in workload parameter " << i
               << ". Expected positive integer, but " << w_params[i].u_param << " received.";
            throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
        } // end else if

    retval.at(0) = w_params.at(0).u_param; // n
    retval.at(1) = w_params.at(1).u_param; // m
    retval.at(2) = w_params.at(2).u_param; // k

    return retval;
}

void BenchmarkDescriptorCategory::completeWorkloadDescription(WorkloadDescriptionOutput &output,
                                                              const Engine &engine,
                                                              const BenchmarkDescription::Backend &backend_desc,
                                                              const BenchmarkDescription::Configuration &config) const
{
    (void)engine;
    std::stringstream ss;

    output.concrete_descriptor = backend_desc.descriptor;
    PartialBenchmarkDescriptor::completeCategoryParams(output.concrete_descriptor,
                                                       backend_desc.descriptor,
                                                       config,
                                                       PartialBenchmarkDescriptor::getForceConfigValues());

    // workload name

    auto sets_size = fetchSetSize(config.w_params);
    ss << BaseWorkloadName
       << " |X| -> " << sets_size[0] << ", "
       << "|Y| -> " << sets_size[1] << ", "
       << "k  -> " << sets_size[2];

    output.workload_name          = ss.str();
    output.operation_params_count = BenchmarkDescriptorCategory::OpParameterCount;
}

bool BenchmarkDescriptorCategory::matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                                           const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const
{
    bool retval = false;

    // return true if benchmark is supported
    if (bench_desc.workload == hebench::APIBridge::Workload::SimpleSetIntersection)
    {
        try
        {
            fetchSetSize(w_params);
            retval = true;
        }
        catch (...)
        {
            // workload not supported
            retval = false;
        }
    } // end if

    return retval;
}

//---------------------------
// class DataGeneratorHelper
//---------------------------

/**
 * @brief Static helper class to generate vector data for all supported data types.
 */
class DataGeneratorHelper : public hebench::TestHarness::DataGeneratorHelper
{
private:
    IL_DECLARE_CLASS_NAME(SimpleSetIntersection::DataGeneratorHelper)

public:
    static void vectorSimpleSetIntersection(hebench::APIBridge::DataType data_type,
                                            void *result, const void *x, const void *y,
                                            std::uint64_t n, std::uint64_t m, std::uint64_t k);

protected:
    DataGeneratorHelper() {}

private:
    template <class T>
    static bool isMemberOf(const T *dataset, const T *value, std::uint64_t n, std::uint64_t k)
    {
        bool retval = false;
        for (size_t i = 0; !retval && i < n; ++i)
        {
            std::uint64_t members = 0;
            bool flag             = true;
            for (size_t j = 0; flag && j < k; ++j)
            {
                flag = dataset[(i * k) + j] == value[j];
                if (flag)
                {
                    ++members;
                }
            }
            retval = members == k;
        }
        return retval;
    }

    template <class T>
    static void mySetIntersection(T *result, const T *dataset_X, const T *dataset_Y, std::uint64_t n, std::uint64_t m, std::uint64_t k)
    {
        size_t idx_result = 0;

        // initialize result with all zeros
        std::fill(result, result + m * k, static_cast<T>(0));

        for (size_t idx_x = 0; idx_x < n; ++idx_x)
        {
            if (isMemberOf(dataset_Y, dataset_X + (idx_x * k), m, k))
            {
                // check for duplicates
                if (!isMemberOf(result, dataset_X + (idx_x * k), m, k))
                {
                    std::copy(dataset_X + (idx_x * k),
                              dataset_X + (idx_x * k) + k,
                              result + (idx_result * k));
                    ++idx_result;
                } // end if
            }
        }
    }

    template <class T>
    static void vectorSimpleSetIntersection(T *result, const T *x, const T *y, std::uint64_t n, std::uint64_t m, std::uint64_t k)
    {
        if (!x)
            throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null `x`"));
        if (!y)
            throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null `y`"));
        if (k == 0)
            throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null `k`"));

        if (n > m)
        {
            mySetIntersection(result, x, y, n, m, k);
        }
        else
        {
            mySetIntersection(result, y, x, m, n, k);
        }
    }
};

void DataGeneratorHelper::vectorSimpleSetIntersection(hebench::APIBridge::DataType data_type,
                                                      void *result, const void *x, const void *y,
                                                      std::uint64_t n, std::uint64_t m, std::uint64_t k)
{
    if (!result)
    {
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null 'p_result'."));
    }

    switch (data_type)
    {
    case hebench::APIBridge::DataType::Int32:
        vectorSimpleSetIntersection<std::int32_t>(reinterpret_cast<std::int32_t *>(result),
                                                  reinterpret_cast<const std::int32_t *>(x), reinterpret_cast<const std::int32_t *>(y),
                                                  n, m, k);
        break;

    case hebench::APIBridge::DataType::Int64:
        vectorSimpleSetIntersection<std::int64_t>(reinterpret_cast<std::int64_t *>(result),
                                                  reinterpret_cast<const std::int64_t *>(x), reinterpret_cast<const std::int64_t *>(y),
                                                  n, m, k);
        break;

    case hebench::APIBridge::DataType::Float32:
        vectorSimpleSetIntersection<float>(reinterpret_cast<float *>(result),
                                           reinterpret_cast<const float *>(x), reinterpret_cast<const float *>(y),
                                           n, m, k);
        break;

    case hebench::APIBridge::DataType::Float64:
        vectorSimpleSetIntersection<double>(reinterpret_cast<double *>(result),
                                            reinterpret_cast<const double *>(x), reinterpret_cast<const double *>(y),
                                            n, m, k);
        break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Unknown data type."));
        break;
    } // end switch
}

//------------------
// class DataLoader
//------------------

DataLoader::Ptr DataLoader::create(std::uint64_t set_size_x,
                                   std::uint64_t set_size_y,
                                   std::uint64_t batch_size_x,
                                   std::uint64_t batch_size_y,
                                   std::uint64_t element_size_k,
                                   hebench::APIBridge::DataType data_type)
{
    DataLoader::Ptr retval = DataLoader::Ptr(new DataLoader());
    retval->init(set_size_x, set_size_y, batch_size_x, batch_size_y, element_size_k, data_type);
    return retval;
}

DataLoader::Ptr DataLoader::create(std::uint64_t set_size_x,
                                   std::uint64_t set_size_y,
                                   std::uint64_t batch_size_x,
                                   std::uint64_t batch_size_y,
                                   std::uint64_t element_size_k,
                                   hebench::APIBridge::DataType data_type,
                                   const std::string &dataset_filename)
{
    DataLoader::Ptr retval = DataLoader::Ptr(new DataLoader());
    retval->init(set_size_x, set_size_y, batch_size_x, batch_size_y, element_size_k, data_type, dataset_filename);
    return retval;
}

DataLoader::DataLoader() :
    m_set_size_x(0), m_set_size_y(0), m_element_size_k(1)
{
}

void DataLoader::init(std::uint64_t set_size_x,
                      std::uint64_t set_size_y,
                      std::uint64_t batch_size_x,
                      std::uint64_t batch_size_y,
                      std::uint64_t element_size_k,
                      hebench::APIBridge::DataType data_type)
{
    // Load/generate and initialize the data for vectors to be applied for simple set intersection:
    // Z = Intersect(X, Y)

    // number of samples in each input parameter and output
    std::size_t batch_sizes[InputDim0 + OutputDim0] = {
        batch_size_x,
        batch_size_y,
        (batch_size_x * batch_size_y)
    };

    m_set_size_x     = set_size_x;
    m_set_size_y     = set_size_y;
    m_element_size_k = element_size_k;

    // compute buffer size in bytes for each set (vector)
    std::uint64_t sample_set_sizes[InputDim0 + OutputDim0] = {
        set_size_x * element_size_k,
        set_size_y * element_size_k,
        std::min(set_size_x, set_size_y) * element_size_k // Assuming there is a set that contains the other
    };

    // initialize data packs and allocate memory
    PartialDataLoader::init(data_type,
                            InputDim0, batch_sizes, sample_set_sizes,
                            OutputDim0, sample_set_sizes + InputDim0,
                            true);

    // at this point all NativeDataBuffers have been allocated and pointed to the correct locations

    // fill up each set (vector) data

    constexpr std::size_t Param_SetX = 0;
    constexpr std::size_t Param_SetY = 1;

    // generate setX data
    for (std::uint64_t sample_i = 0; sample_i < batch_sizes[Param_SetX]; ++sample_i)
    {
        DataGeneratorHelper::generateRandomVectorU(data_type,
                                                   getParameterData(Param_SetX).p_buffers[sample_i].p,
                                                   sample_set_sizes[Param_SetX],
                                                   -16384.0, 16384.0);
    } // end for

    // generate setY data

    for (std::uint64_t sample_i = 0; sample_i < batch_sizes[Param_SetY]; ++sample_i)
    {
        std::vector<std::uint64_t> indices_y = DataGeneratorHelper::generateRandomIntersectionIndicesU(set_size_y);
        std::vector<std::uint64_t> indices_x;
        std::vector<std::uint64_t>::iterator it_indices_y;
        if (!indices_y.empty())
        {
            // if indices_y is empty, there's no point to execute the following statements
            std::sort(indices_y.begin(), indices_y.end());
            // This will randomly select indices in X, same amount than the ones in indices_y.
            indices_x    = DataGeneratorHelper::generateRandomIntersectionIndicesU(set_size_x, indices_y.size());
            it_indices_y = indices_y.begin();
        }
        // find which sample from X to copy
        std::uint64_t sample_from_X = DataGeneratorHelper::generateRandomIntU(0, getParameterData(Param_SetX).buffer_count - 1);
        std::uint8_t *p_setX        = reinterpret_cast<std::uint8_t *>(getParameterData(Param_SetX).p_buffers[sample_from_X].p);
        std::uint8_t *p_setY        = reinterpret_cast<std::uint8_t *>(getParameterData(Param_SetY).p_buffers[sample_i].p);
        for (std::uint64_t item_y = 0; item_y < set_size_y; ++item_y)
        {
            std::uint8_t *p_setY_item = p_setY + item_y * element_size_k * sizeOf(data_type);
            if (!indices_x.empty() && item_y == *it_indices_y)
            {
                // we found a common item to set
                std::uint8_t *p_setX_item = p_setX + indices_x.back() * element_size_k * sizeOf(data_type);
                std::copy(p_setX_item, p_setX_item + element_size_k * sizeOf(data_type),
                          p_setY_item);
                indices_x.pop_back();
                ++it_indices_y;
            } // end if
            else
            {
                // generate (possibly unique item)
                DataGeneratorHelper::generateRandomVectorU(data_type,
                                                           p_setY_item,
                                                           element_size_k,
                                                           -16384.0, 16384.0);
            } // end else
        } // end for
    } // end for

    for (std::uint64_t sampleX_i = 0; sampleX_i < batch_sizes[Param_SetX]; ++sampleX_i)
        for (std::uint64_t sampleY_i = 0; sampleY_i < batch_sizes[Param_SetY]; ++sampleY_i)
        {
            // find the index for the result buffer based on the input indices
            std::uint64_t ppi[] = { sampleX_i, sampleY_i };
            std::uint64_t z_i   = getResultIndex(ppi);

            // generate the data
            DataGeneratorHelper::vectorSimpleSetIntersection(data_type,
                                                             getResultData(0).p_buffers[z_i].p, // Z
                                                             getParameterData(0).p_buffers[sampleX_i].p, // X
                                                             getParameterData(1).p_buffers[sampleY_i].p, // Y
                                                             set_size_x, set_size_y, element_size_k);
        } // end for

    // all data has been generated at this point
}

void DataLoader::init(std::uint64_t set_size_x,
                      std::uint64_t set_size_y,
                      std::uint64_t batch_size_x,
                      std::uint64_t batch_size_y,
                      std::uint64_t element_size_k,
                      hebench::APIBridge::DataType data_type,
                      const std::string &dataset_filename)
{
    // Load/generate and initialize the data for simple set intersection:
    // Z = X ∩ Y

    // number of samples in each input parameter and output
    std::size_t batch_sizes[InputDim0 + OutputDim0] = {
        batch_size_x,
        batch_size_y,
        (batch_size_x * batch_size_y)
    };

    m_set_size_x     = set_size_x;
    m_set_size_y     = set_size_y;
    m_element_size_k = element_size_k;

    // compute buffer size in bytes for each set (vector)
    std::uint64_t sample_set_sizes[InputDim0 + OutputDim0] = {
        set_size_x * element_size_k,
        set_size_y * element_size_k,
        std::min(set_size_x, set_size_y) * element_size_k // Assuming there is a set that contains the other
    };

    // allocate memory for each set (vector) buffer
    PartialDataLoader::init(dataset_filename, data_type,
                            InputDim0, batch_sizes, sample_set_sizes,
                            OutputDim0, sample_set_sizes + InputDim0);

    // at this point all NativeDataBuffers have been allocated and pointed to the correct locations
    // and buffers loaded with data from dataset_filename
}

void DataLoader::computeResult(std::vector<hebench::APIBridge::NativeDataBuffer *> &result,
                               const std::uint64_t *param_data_pack_indices,
                               hebench::APIBridge::DataType data_type)
{
    // as protected method, parameters should be valid when called

    // generate the output
    DataGeneratorHelper::vectorSimpleSetIntersection(data_type,
                                                     result.front()->p, // Z
                                                     getParameterData(0).p_buffers[param_data_pack_indices[0]].p, // X
                                                     getParameterData(1).p_buffers[param_data_pack_indices[1]].p, // Y
                                                     m_set_size_x, m_set_size_y, m_element_size_k);
}

bool validateResult(IDataLoader::Ptr dataset,
                    const std::uint64_t *param_data_pack_indices,
                    const std::vector<hebench::APIBridge::NativeDataBuffer *> &outputs,
                    std::uint64_t k_count,
                    hebench::APIBridge::DataType data_type)
{
    static constexpr const std::size_t MaxErrorPrint = 10;
    bool retval                                      = true;
    //std::vector<std::uint64_t> is_valid;
    // true => truth has extra element (not in output)
    // false => output has extra element (not in truth)
    std::vector<std::pair<bool, std::uint64_t>> is_valid;

    // extract the pointers to the actual results

    if (outputs.size() != dataset->getResultCount())
    {
        throw std::invalid_argument(IL_LOG_MSG("Invalid number of outputs: 'outputs'."));
    }

    IDataLoader::ResultDataPtr ptr_truths = dataset->getResultFor(param_data_pack_indices);
    const std::vector<const hebench::APIBridge::NativeDataBuffer *> &truths =
        ptr_truths->result;

    // There's at least 1 element that requires processing.
    if (!truths.empty() && !outputs.empty() && truths.front())
    {
        std::size_t index = 0;
        for (index = 0; retval && index < truths.size(); ++index)
        {
            // in case outputs.size() < truths.size() an exception can be triggered
            try
            {
                if (!outputs.at(index))
                {
                    throw std::invalid_argument(IL_LOG_MSG("Unexpected null output component in: 'outputs[" + std::to_string(index) + "]'."));
                }
            }
            catch (const std::out_of_range &out_of_range)
            {
                throw std::invalid_argument(IL_LOG_MSG("Unexpected out of range index output component in: 'outputs[" + std::to_string(index) + "]'."));
            }

            if (outputs.at(index)->size < truths.at(index)->size)
            {
                throw std::invalid_argument(IL_LOG_MSG("Buffer in outputs is not large enough to contain the expected output: 'outputs[" + std::to_string(index) + "]'."));
            }

            void *p_truth  = truths.at(index)->p;
            void *p_output = outputs.at(index)->p; // single output

            // m is not required since both collections have the same size
            std::uint64_t n = truths.at(index)->size / IDataLoader::sizeOf(data_type) / k_count;

            // validate the results
            switch (data_type)
            {
            case hebench::APIBridge::DataType::Int32:
                is_valid = almostEqualSet(reinterpret_cast<const std::int32_t *>(p_truth),
                                          reinterpret_cast<const std::int32_t *>(p_output),
                                          n, n, k_count,
                                          0.01);
                break;

            case hebench::APIBridge::DataType::Int64:
                is_valid = almostEqualSet(reinterpret_cast<const std::int64_t *>(p_truth),
                                          reinterpret_cast<const std::int64_t *>(p_output),
                                          n, n, k_count,
                                          0.01);
                break;

            case hebench::APIBridge::DataType::Float32:
                is_valid = almostEqualSet(reinterpret_cast<const float *>(p_truth),
                                          reinterpret_cast<const float *>(p_output),
                                          n, n, k_count,
                                          0.01);
                break;

            case hebench::APIBridge::DataType::Float64:
                is_valid = almostEqualSet(reinterpret_cast<const double *>(p_truth),
                                          reinterpret_cast<const double *>(p_output),
                                          n, n, k_count,
                                          0.01);
                break;

            default:
                retval = false;
                break;
            } // end switch

            // In case retval is set to false, it will break the for loop
            retval = retval && is_valid.empty();
        } // end for

        if (!retval)
        {
            std::stringstream ss;
            ss << "Result component, " << (index - 1) << std::endl
               << "Elements mismatched, " << is_valid.size() << std::endl
               << "Failed indices, ";
            for (std::size_t i = 0; i < is_valid.size() && i < MaxErrorPrint; ++i)
            {
                ss << "(" << (is_valid[i].first ? "ground truth" : "output") << "; " << is_valid[i].second * k_count << ".." << is_valid[i].second * k_count + k_count - 1 << ")";
                if (i + 1 < is_valid.size() && i + 1 < MaxErrorPrint)
                {
                    ss << ", ";
                }
            } // end for
            if (is_valid.size() > MaxErrorPrint)
            {
                ss << ", ...";
            }
            throw std::runtime_error(ss.str());
        } // end if
    } // end if

    return retval;
}

} // namespace SimpleSetIntersection
} // namespace TestHarness
} // namespace hebench
