
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <sstream>

#include "../include/hebench_genericwl.h"

namespace hebench {
namespace TestHarness {
namespace GenericWL {

//------------------------------------
// class BenchmarkDescriptionCategory
//------------------------------------

std::pair<std::vector<std::uint64_t>, std::vector<std::uint64_t>>
BenchmarkDescriptorCategory::fetchIOVectorSizes(const std::vector<hebench::APIBridge::WorkloadParam> &w_params)
{
    std::pair<std::vector<std::uint64_t>, std::vector<std::uint64_t>> retval;

    if (w_params.size() < WorkloadParameterMinCount)
    {
        std::stringstream ss;
        ss << "Insufficient workload parameters in 'w_params'. Expected, at least, " << WorkloadParameterMinCount
           << ", but " << w_params.size() << "received.";
        throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
    } // end if
    for (std::size_t i = 0; i < w_params.size(); ++i)
    {
        if (w_params[i].data_type != WorkloadParameterType)
        {
            std::stringstream ss;
            ss << "Invalid type for workload parameter " << i
               << ". Expected type ID " << WorkloadParameterType << ", but " << w_params[i].data_type << " received.";
            throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
        } // end if
        else if (w_params[i].u_param <= 0)
        {
            std::stringstream ss;
            ss << "Invalid value in workload parameter " << i
               << ". Expected positive integer, but " << w_params[i].u_param << " received.";
            throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
        } // end if
    } // end for

    std::vector<std::uint64_t> &input_sizes  = retval.first;
    std::vector<std::uint64_t> &output_sizes = retval.second;

    input_sizes.resize(w_params[0].u_param);
    output_sizes.resize(w_params[1].u_param);

    const std::size_t NumWorkloadParams = input_sizes.size() + output_sizes.size() + 2;

    if (input_sizes.size() > HEBENCH_MAX_OP_PARAMS)
    {
        std::stringstream ss;
        ss << "Number of input parameters for the operation must be, at most, " << HEBENCH_MAX_OP_PARAMS
           << ", but " << input_sizes.size() << "received.";
        throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
    } // end if

    if (w_params.size() < NumWorkloadParams)
    {
        std::stringstream ss;
        ss << "Insufficient workload parameters in 'w_params'. Expected " << NumWorkloadParams
           << ", but " << w_params.size() << "received.";
        throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
    } // end if

    for (std::size_t i = 0; i < input_sizes.size(); ++i)
        input_sizes[i] = w_params[i + 2].u_param;
    for (std::size_t i = 0; i < output_sizes.size(); ++i)
        output_sizes[i] = w_params[input_sizes.size() + i + 2].u_param;

    return retval;
}

bool BenchmarkDescriptorCategory::matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                                           const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const
{
    bool retval = false;

    // return true if benchmark is supported
    if (bench_desc.workload == hebench::APIBridge::Workload::Generic)
    {
        try
        {
            fetchIOVectorSizes(w_params);
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

    auto op_info = fetchIOVectorSizes(config.w_params);
    ss << BaseWorkloadName << ", " << op_info.first.size() << " Inputs, " << op_info.second.size() << " Outputs";

    output.workload_name          = ss.str();
    output.operation_params_count = op_info.first.size();
}

//---------------------
// class DataLoader
//---------------------

DataLoader::Ptr DataLoader::create(const std::vector<std::uint64_t> &input_sizes,
                                   const std::vector<std::uint64_t> &max_batch_sizes,
                                   const std::vector<std::uint64_t> &output_sizes,
                                   hebench::APIBridge::DataType data_type,
                                   const std::string &dataset_filename)
{
    DataLoader::Ptr retval = DataLoader::Ptr(new DataLoader());
    retval->init(input_sizes, max_batch_sizes, output_sizes, data_type, dataset_filename);
    return retval;
}

DataLoader::DataLoader()
{
}

void DataLoader::init(const std::vector<std::uint64_t> &input_sizes,
                      const std::vector<std::uint64_t> &max_batch_sizes,
                      const std::vector<std::uint64_t> &output_sizes,
                      hebench::APIBridge::DataType data_type,
                      const std::string &dataset_filename)
{
    // Load the data for generic operation:

    if (input_sizes.size() != max_batch_sizes.size())
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Number of elements in `input_sizes` must match number of elements in `max_batch_sizes`."));

    PartialDataLoader::init(dataset_filename, data_type,
                            input_sizes.size(),
                            max_batch_sizes.data(),
                            input_sizes.data(),
                            output_sizes.size(),
                            output_sizes.data());

    // at this point all NativeDataBuffers have been allocated, pointed to the correct locations
    // and filled with data from the specified dataset file
}

void DataLoader::computeResult(std::vector<hebench::APIBridge::NativeDataBuffer *> &result,
                               const std::uint64_t *param_data_pack_indices,
                               hebench::APIBridge::DataType data_type)
{
    // as protected method, parameters should be valid when called

    (void)result;
    (void)param_data_pack_indices;
    (void)data_type;

    // cannot generate output
    throw std::logic_error(IL_LOG_MSG_CLASS("Unable to compute output for generic workload operation."));
}

} // namespace GenericWL
} // namespace TestHarness
} // namespace hebench
