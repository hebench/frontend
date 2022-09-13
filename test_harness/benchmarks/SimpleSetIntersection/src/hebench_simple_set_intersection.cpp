
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <numeric>
#include <sstream>

#include "../include/hebench_simple_set_intersection.h"
#include "modules/general/include/hebench_math_utils.h"

namespace hebench {
namespace TestHarness {
namespace SimpleSetIntersection {

//------------------------------------
// class BenchmarkDescriptionCategory
//------------------------------------

hebench::APIBridge::WorkloadParamType::WorkloadParamType
    BenchmarkDescriptorCategory::WorkloadParameterType[BenchmarkDescriptorCategory::WorkloadParameterCount] = {
        hebench::APIBridge::WorkloadParamType::UInt64,
        hebench::APIBridge::WorkloadParamType::UInt64
    };

// EZR: this needs to return how many items in dataset X, in dataset Y, and k (number of elements in each dataset item)
//      std::array<std::uint64_t, 3>
std::array<std::uint64_t, BenchmarkDescriptorCategory::OpParameterCount> BenchmarkDescriptorCategory::fetchSetSize(const std::vector<hebench::APIBridge::WorkloadParam> &w_params)
{
    // EZR: WorkloadParameterCount should be 3 (see header)
    assert(WorkloadParameterCount == 2);
    assert(OpParameterCount == 2);
    assert(OpResultCount == 1);

    std::array<std::uint64_t, OpParameterCount> retval;

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

    retval.at(0) = w_params.at(0).u_param;
    retval.at(1) = w_params.at(1).u_param;

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
    if (output.concrete_descriptor.cat_params.min_test_time_ms == 0)
        output.concrete_descriptor.cat_params.min_test_time_ms = config.default_min_test_time_ms;

    // workload name

    auto sets_size = fetchSetSize(config.w_params);
    ss << BaseWorkloadName
       << " |X| -> " << sets_size[0] << ", "
       << " |Y| -> " << sets_size[1];

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
                                            std::uint64_t n, std::uint64_t m);

protected:
    DataGeneratorHelper() {}

private:
    template <class T>
    static void vectorSimpleSetIntersection(T *result, const T *x, const T *y, std::uint64_t n, std::uint64_t m)
    {
        if (!x)
            throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null `x`"));
        if (!y)
            throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null `y`"));

        T *sorted_x = const_cast<T *>(x);
        T *sorted_y = const_cast<T *>(y);
        std::sort(sorted_x, sorted_x + n);
        std::sort(sorted_y, sorted_y + m);
        std::set_intersection(sorted_x, sorted_x + n, sorted_y, sorted_y + m, result);
    }
};

void DataGeneratorHelper::vectorSimpleSetIntersection(hebench::APIBridge::DataType data_type,
                                                      void *result, const void *x, const void *y,
                                                      std::uint64_t n, std::uint64_t m)
{
    if (!result)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null 'p_result'."));

    switch (data_type)
    {
    case hebench::APIBridge::DataType::Int32:
        vectorSimpleSetIntersection<std::int32_t>(reinterpret_cast<std::int32_t *>(result),
                                                  reinterpret_cast<const std::int32_t *>(x), reinterpret_cast<const std::int32_t *>(y),
                                                  n, m);
        break;

    case hebench::APIBridge::DataType::Int64:
        vectorSimpleSetIntersection<std::int64_t>(reinterpret_cast<std::int64_t *>(result),
                                                  reinterpret_cast<const std::int64_t *>(x), reinterpret_cast<const std::int64_t *>(y),
                                                  n, m);
        break;

    case hebench::APIBridge::DataType::Float32:
        vectorSimpleSetIntersection<float>(reinterpret_cast<float *>(result),
                                           reinterpret_cast<const float *>(x), reinterpret_cast<const float *>(y),
                                           n, m);
        break;

    case hebench::APIBridge::DataType::Float64:
        vectorSimpleSetIntersection<double>(reinterpret_cast<double *>(result),
                                            reinterpret_cast<const double *>(x), reinterpret_cast<const double *>(y),
                                            n, m);
        break;

    // TODO: ask if this is required
    case hebench::APIBridge::DataType::String:
        vectorSimpleSetIntersection<std::string>(reinterpret_cast<std::string *>(result),
                                                 reinterpret_cast<const std::string *>(x), reinterpret_cast<const std::string *>(y),
                                                 n, m);
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
                                   hebench::APIBridge::DataType data_type)
{
    DataLoader::Ptr retval = DataLoader::Ptr(new DataLoader());
    retval->init(set_size_x, set_size_y, batch_size_x, batch_size_y, data_type);
    return retval;
}

DataLoader::Ptr DataLoader::create(std::uint64_t set_size_x,
                                   std::uint64_t set_size_y,
                                   std::uint64_t batch_size_x,
                                   std::uint64_t batch_size_y,
                                   hebench::APIBridge::DataType data_type,
                                   const std::string &dataset_filename)
{
    DataLoader::Ptr retval = DataLoader::Ptr(new DataLoader());
    retval->init(set_size_x, set_size_y, batch_size_x, batch_size_y, data_type, dataset_filename);
    return retval;
}

DataLoader::DataLoader() :
    m_set_size_x(0), m_set_size_y(0)
{
}

void DataLoader::init(std::uint64_t set_size_x,
                      std::uint64_t set_size_y,
                      std::uint64_t batch_size_x,
                      std::uint64_t batch_size_y,
                      hebench::APIBridge::DataType data_type)
{
    // Load/generate and initialize the data for vectors to be applied for simple set intersection:
    // Z = Intersect(X, Y)

    // number of samples in each input parameter and output
    std::size_t batch_sizes[InputDim0 + OutputDim0] = {
        batch_size_x,
        batch_size_y,
        batch_size_x * batch_size_y
    };

    m_set_size_x = set_size_x;
    m_set_size_y = set_size_y;

    // compute buffer size in bytes for each set (vector)
    std::uint64_t sample_set_sizes[InputDim0 + OutputDim0] = {
        set_size_x,
        set_size_y,
        std::min(set_size_x, set_size_y) // Assuming there is a set that contains the other
    };

    // initialize data packs and allocate memory
    PartialDataLoader::init(data_type,
                            InputDim0, batch_sizes, sample_set_sizes,
                            OutputDim0, sample_set_sizes + InputDim0,
                            true);

    // at this point all NativeDataBuffers have been allocated and pointed to the correct locations

    // fill up each set (vector) data

    // input
    std::uint64_t set_size = 0;
    for (std::size_t set_i = 0; set_i < InputDim0; ++set_i)
    {
        for (std::uint64_t i = 0; i < batch_sizes[set_i]; ++i)
        {
            // in case the batch size differs from 1
            set_size = (set_i % 2 == 0) ? set_size_x : set_size_y;
            // generate the data
            DataGeneratorHelper::generateRandomSetN(data_type,
                                                    getParameterData(set_i).p_buffers[i].p,
                                                    set_size, 0.0, 10.0);
        } // end for
    } // end for

    // output
    //#pragma omp parallel for collapse(2)
    for (std::uint64_t input_i = 0; input_i < batch_sizes[2]; ++input_i)
    {
        // find the index for the result buffer based on the input indices
        std::uint64_t ppi[] = { 0, 0, input_i };
        std::uint64_t z_i   = getResultIndex(ppi);

        // generate the data
        DataGeneratorHelper::vectorSimpleSetIntersection(data_type,
                                                         getResultData(0).p_buffers[z_i].p, // Z
                                                         getParameterData(0).p_buffers[input_i].p, // X
                                                         getParameterData(1).p_buffers[input_i].p, // Y
                                                         set_size_x, set_size_y);
    } // end for

    // all data has been generated at this point
}

void DataLoader::init(std::uint64_t set_size_x,
                      std::uint64_t set_size_y,
                      std::uint64_t batch_size_x,
                      std::uint64_t batch_size_y,
                      hebench::APIBridge::DataType data_type,
                      const std::string &dataset_filename)
{
    // Load/generate and initialize the data for simple set intersection:
    // Z = X ∩ Y

    // number of samples in each input parameter and output
    std::size_t batch_sizes[InputDim0 + OutputDim0] = {
        batch_size_x,
        batch_size_y,
        batch_size_x * batch_size_y
    };

    m_set_size_x = set_size_x;
    m_set_size_y = set_size_y;

    // compute buffer size in bytes for each set (vector)
    std::uint64_t sample_set_sizes[InputDim0 + OutputDim0] = {
        set_size_x,
        set_size_y,
        std::min(set_size_x, set_size_y) // Assuming there is a set that contains the other
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
                                                     m_set_size_x, m_set_size_y);
}

} // namespace SimpleSetIntersection
} // namespace TestHarness
} // namespace hebench
