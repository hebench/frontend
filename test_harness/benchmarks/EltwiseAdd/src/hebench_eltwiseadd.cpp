
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <sstream>

#include "../include/hebench_eltwiseadd.h"

namespace hebench {
namespace TestHarness {
namespace EltwiseAdd {

//------------------------------------
// class BenchmarkDescriptionCategory
//------------------------------------

hebench::APIBridge::WorkloadParamType::WorkloadParamType
    BenchmarkDescriptorCategory::WorkloadParameterType[BenchmarkDescriptorCategory::WorkloadParameterCount] = {
        hebench::APIBridge::WorkloadParamType::UInt64
    };

std::uint64_t BenchmarkDescriptorCategory::fetchVectorSize(const std::vector<hebench::APIBridge::WorkloadParam> &w_params)
{
    assert(WorkloadParameterCount == 1);
    assert(OpParameterCount == 2);
    assert(OpResultCount == 1);

    std::uint64_t retval;

    if (w_params.size() < WorkloadParameterCount)
    {
        std::stringstream ss;
        ss << "Insufficient workload parameters in 'w_params'. Expected " << WorkloadParameterCount
           << ", but " << w_params.size() << "received.";
        throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
    } // end if

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
        } // end if

    retval = w_params.at(0).u_param;

    return retval;
}

bool BenchmarkDescriptorCategory::matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                                           const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const
{
    bool retval = false;

    // return true if benchmark is supported
    if (bench_desc.workload == hebench::APIBridge::Workload::EltwiseAdd)
    {
        try
        {
            fetchVectorSize(w_params);
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

void BenchmarkDescriptorCategory::completeWorkloadDescription(WorkloadDescriptionOutput &output,
                                                              const Engine &engine,
                                                              const BenchmarkDescription::Backend &backend_desc,
                                                              const BenchmarkDescription::Configuration &config) const
{
    (void)engine;
    (void)backend_desc;
    std::stringstream ss;

    // workload name

    std::uint64_t vector_size = fetchVectorSize(config.w_params);
    ss << BaseWorkloadName << " " << vector_size;

    output.workload_name          = ss.str();
    output.operation_params_count = BenchmarkDescriptorCategory::OpParameterCount;
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
    IL_DECLARE_CLASS_NAME(EltwiseAdd::DataGeneratorHelper)

public:
    static void vectorEltwiseAdd(hebench::APIBridge::DataType data_type,
                                 void *result, const void *a, const void *b,
                                 std::uint64_t elem_count);

protected:
    DataGeneratorHelper() {}

private:
    template <class T>
    static void vectorEltwiseAdd(T *result, const T *a, const T *b, std::uint64_t elem_count)
    {
        if (!result)
            throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null `result`"));
        if (!a)
            throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null `a`"));
        if (!b)
            throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null `b`"));
        std::transform(a, a + elem_count, // a[0..elem_count]
                       b, // b[0..elem_count]
                       result, // result[0..elem_count]
                       std::plus<T>()); // eltwise add
    }
};

void DataGeneratorHelper::vectorEltwiseAdd(hebench::APIBridge::DataType data_type,
                                           void *result, const void *a, const void *b,
                                           std::uint64_t elem_count)
{
    switch (data_type)
    {
    case hebench::APIBridge::DataType::Int32:
        vectorEltwiseAdd<std::int32_t>(reinterpret_cast<std::int32_t *>(result),
                                       reinterpret_cast<const std::int32_t *>(a), reinterpret_cast<const std::int32_t *>(b),
                                       elem_count);
        break;

    case hebench::APIBridge::DataType::Int64:
        vectorEltwiseAdd<std::int64_t>(reinterpret_cast<std::int64_t *>(result),
                                       reinterpret_cast<const std::int64_t *>(a), reinterpret_cast<const std::int64_t *>(b),
                                       elem_count);
        break;

    case hebench::APIBridge::DataType::Float32:
        vectorEltwiseAdd<float>(reinterpret_cast<float *>(result),
                                reinterpret_cast<const float *>(a), reinterpret_cast<const float *>(b),
                                elem_count);
        break;

    case hebench::APIBridge::DataType::Float64:
        vectorEltwiseAdd<double>(reinterpret_cast<double *>(result),
                                 reinterpret_cast<const double *>(a), reinterpret_cast<const double *>(b),
                                 elem_count);
        break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Unknown data type."));
        break;
    } // end switch
}

//---------------------
// class DataGenerator
//---------------------

DataLoader::Ptr DataLoader::create(std::uint64_t vector_size,
                                   std::uint64_t batch_size_a,
                                   std::uint64_t batch_size_b,
                                   hebench::APIBridge::DataType data_type)
{
    DataLoader::Ptr retval = DataLoader::Ptr(new DataLoader());
    retval->init(vector_size, batch_size_a, batch_size_b, data_type);
    return retval;
}

DataLoader::Ptr DataLoader::create(std::uint64_t expected_vector_size,
                                   std::uint64_t max_batch_size_a,
                                   std::uint64_t max_batch_size_b,
                                   hebench::APIBridge::DataType data_type,
                                   const std::string &dataset_filename)
{
    DataLoader::Ptr retval = DataLoader::Ptr(new DataLoader());
    retval->init(expected_vector_size, max_batch_size_a, max_batch_size_b, data_type, dataset_filename);
    return retval;
}

DataLoader::DataLoader() :
    m_vector_size(0)
{
}

void DataLoader::init(std::uint64_t vector_size,
                      std::uint64_t batch_size_a,
                      std::uint64_t batch_size_b,
                      hebench::APIBridge::DataType data_type)
{
    // Load/generate and initialize the data for vector element-wise addition:
    // C = A + B

    // number of samples in each input parameter and output
    std::size_t batch_sizes[InputDim0 + OutputDim0] = {
        batch_size_a,
        batch_size_b,
        batch_size_a * batch_size_b
    };

    // compute buffer size in bytes for each vector
    std::vector<std::uint64_t> sample_vector_sizes(InputDim0 + OutputDim0, vector_size);
    m_vector_size = vector_size;

    PartialDataLoader::init(data_type,
                            InputDim0, // number of input parameters
                            batch_sizes, // samples per parameter
                            sample_vector_sizes.data(), // vector size for each parameter
                            OutputDim0, // number of result components
                            sample_vector_sizes.data() + InputDim0, // number of result samples
                            true); // allocate memory for ground truth?

    // at this point all NativeDataBuffers have been allocated and pointed to the correct locations

    // fill up each vector data

    // input
    for (std::size_t vector_i = 0; vector_i < InputDim0; ++vector_i)
    {
        for (std::uint64_t i = 0; i < batch_sizes[vector_i]; ++i)
        {
            // generate the data
            DataGeneratorHelper::generateRandomVectorU(data_type,
                                                       getParameterData(vector_i).p_buffers[i].p,
                                                       vector_size, -10.0, 10.0);
        } // end for
    } // end for

    // output
    //#pragma omp parallel for collapse(2)
    for (std::uint64_t a_i = 0; a_i < batch_sizes[0]; ++a_i)
    {
        for (std::uint64_t b_i = 0; b_i < batch_sizes[1]; ++b_i)
        {
            // find the index for the result buffer based on the input indices
            std::uint64_t ppi[] = { a_i, b_i };
            std::uint64_t r_i   = getResultIndex(ppi);

            // generate the data
            DataGeneratorHelper::vectorEltwiseAdd(data_type,
                                                  getResultData(0).p_buffers[r_i].p, // C
                                                  getParameterData(0).p_buffers[a_i].p, // A
                                                  getParameterData(1).p_buffers[b_i].p, // B
                                                  vector_size);
        } // end for
    } // end for

    // all data has been generated at this point
}

void DataLoader::init(std::uint64_t expected_vector_size,
                      std::uint64_t max_batch_size_a,
                      std::uint64_t max_batch_size_b,
                      hebench::APIBridge::DataType data_type,
                      const std::string &dataset_filename)
{
    // Load and initialize the data for vector element-wise addition:
    // C = A + B

    // number of samples in each input parameter and output
    std::size_t batch_sizes[InputDim0 + OutputDim0] = {
        max_batch_size_a,
        max_batch_size_b,
        max_batch_size_a * max_batch_size_b
    };

    // compute buffer size in bytes for each vector
    std::vector<std::uint64_t> sample_vector_sizes(InputDim0 + OutputDim0, expected_vector_size);
    m_vector_size = expected_vector_size;

    PartialDataLoader::init(dataset_filename, data_type,
                            InputDim0,
                            batch_sizes,
                            sample_vector_sizes.data(),
                            OutputDim0,
                            sample_vector_sizes.data() + InputDim0);

    // at this point all NativeDataBuffers have been allocated, pointed to the correct locations
    // and filled with data from the specified dataset file
}

void DataLoader::computeResult(std::vector<hebench::APIBridge::NativeDataBuffer *> &result,
                               const std::uint64_t *param_data_pack_indices,
                               hebench::APIBridge::DataType data_type)
{
    // as protected method, parameters should be valid when called

    // generate the output
    DataGeneratorHelper::vectorEltwiseAdd(data_type,
                                          result.front()->p, // C
                                          this->getParameterData(0).p_buffers[param_data_pack_indices[0]].p, // A
                                          this->getParameterData(1).p_buffers[param_data_pack_indices[1]].p, // B
                                          m_vector_size);
}

} // namespace EltwiseAdd
} // namespace TestHarness
} // namespace hebench
