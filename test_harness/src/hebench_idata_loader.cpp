
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <cassert>
#include <cstring>
#include <functional>
#include <sstream>
#include <stdexcept>

#include "hebench_dataset_loader.h"

#include "../include/hebench_idata_loader.h"

namespace hebench {
namespace TestHarness {

//-------------------
// class IDataLoader
//-------------------

std::size_t IDataLoader::sizeOf(hebench::APIBridge::DataType data_type)
{
    std::size_t retval = 0;

    switch (data_type)
    {
    case hebench::APIBridge::DataType::Int32:
        retval = sizeof(std::int32_t);
        break;

    case hebench::APIBridge::DataType::Int64:
        retval = sizeof(std::int64_t);
        break;

    case hebench::APIBridge::DataType::Float32:
        retval = sizeof(float);
        break;

    case hebench::APIBridge::DataType::Float64:
        retval = sizeof(double);
        break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Unknown data type."));
        break;
    } // end switch

    return retval;
}

IDataLoader::unique_ptr_custom_deleter<hebench::APIBridge::DataPackCollection>
IDataLoader::createDataPackCollection(std::uint64_t data_pack_count)
{
    unique_ptr_custom_deleter<hebench::APIBridge::DataPackCollection> retval;
    hebench::APIBridge::DataPack *p_data_packs = nullptr;

    try
    {
        p_data_packs = new hebench::APIBridge::DataPack[data_pack_count];
        retval       = unique_ptr_custom_deleter<hebench::APIBridge::DataPackCollection>(
            new hebench::APIBridge::DataPackCollection(),
            [](hebench::APIBridge::DataPackCollection *p) {
                if (p)
                {
                    if (p->p_data_packs)
                        delete[] p->p_data_packs;
                    delete p;
                }
            });

        retval->pack_count   = data_pack_count;
        retval->p_data_packs = p_data_packs;
    }
    catch (...)
    {
        if (p_data_packs)
            delete[] p_data_packs;
        throw;
    }

    return retval;
}

IDataLoader::unique_ptr_custom_deleter<hebench::APIBridge::DataPack>
IDataLoader::createDataPack(std::uint64_t buffer_count, std::uint64_t param_position)
{
    unique_ptr_custom_deleter<hebench::APIBridge::DataPack> retval;
    hebench::APIBridge::NativeDataBuffer *p_buffers = nullptr;

    try
    {
        p_buffers = new hebench::APIBridge::NativeDataBuffer[buffer_count];
        retval    = unique_ptr_custom_deleter<hebench::APIBridge::DataPack>(
            new hebench::APIBridge::DataPack(),
            [](hebench::APIBridge::DataPack *p) {
                if (p)
                {
                    if (p->p_buffers)
                        delete[] p->p_buffers;
                    delete p;
                }
            });

        retval->buffer_count   = buffer_count;
        retval->param_position = param_position;
        retval->p_buffers      = p_buffers;
    }
    catch (...)
    {
        if (p_buffers)
            delete[] p_buffers;
        throw;
    }

    return retval;
}

IDataLoader::unique_ptr_custom_deleter<hebench::APIBridge::NativeDataBuffer>
IDataLoader::createDataBuffer(std::uint64_t size, std::int64_t tag)
{
    unique_ptr_custom_deleter<hebench::APIBridge::NativeDataBuffer> retval;
    std::uint8_t *p_buffer = nullptr;

    try
    {
        p_buffer = new std::uint8_t[size];
        retval   = unique_ptr_custom_deleter<hebench::APIBridge::NativeDataBuffer>(
            new hebench::APIBridge::NativeDataBuffer(),
            [](hebench::APIBridge::NativeDataBuffer *p) {
                if (p)
                {
                    if (p->p)
                    {
                        std::uint8_t *p_tmp = reinterpret_cast<std::uint8_t *>(p->p);
                        delete[] p_tmp;
                    }
                    delete p;
                }
            });

        retval->size = size;
        retval->tag  = tag;
        retval->p    = p_buffer;
    }
    catch (...)
    {
        if (p_buffer)
            delete[] p_buffer;
        throw;
    }

    return retval;
}

//-------------------------------
// class PartialDataLoaderHelper
//-------------------------------

template <typename T>
class PartialDataLoaderHelper
{
private:
    IL_DECLARE_CLASS_NAME(PartialDataLoaderHelper)
public:
    static void init(PartialDataLoader &data_loader,
                     std::size_t input_dim,
                     const std::size_t *input_sample_count_per_dim,
                     const std::uint64_t *input_count_per_dim,
                     std::size_t output_dim,
                     const std::uint64_t *output_count_per_dim,
                     bool allocate_output);
    static void loadFromFile(PartialDataLoader &data_loader,
                             const std::string &filename,
                             std::size_t expected_input_dim,
                             const std::size_t *max_input_sample_count_per_dim,
                             const std::uint64_t *expected_input_count_per_dim,
                             std::size_t expected_output_dim,
                             const std::uint64_t *expected_output_count_per_dim);
};

template <typename T>
inline void PartialDataLoaderHelper<T>::init(PartialDataLoader &data_loader,
                                             std::size_t input_dim,
                                             const std::size_t *input_sample_count_per_dim,
                                             const std::uint64_t *input_count_per_dim,
                                             std::size_t output_dim,
                                             const std::uint64_t *output_count_per_dim,
                                             bool allocate_output)
{
    if (input_dim <= 0)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid input dimensions: 'input_dim' must be positive."));
    if (output_dim <= 0)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid output dimensions: 'output_dim' must be positive."));

    data_loader.m_input_data.resize(input_dim);
    data_loader.m_output_data.resize(output_dim);

    std::size_t output_sample_count_per_dim = 1;
    for (std::size_t i = 0; i < data_loader.m_input_data.size(); ++i)
    {
        if (input_sample_count_per_dim[i] <= 0)
        {
            std::stringstream ss;
            ss << "Invalid batch size for dimension " << i << ": 'input_count_per_dim[" << i << "]' must be positive.";
            throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
        } // end if

        output_sample_count_per_dim *= input_sample_count_per_dim[i];
        data_loader.m_input_data[i] = data_loader.createDataPack(input_sample_count_per_dim[i], i);
    } // end for

    for (std::size_t i = 0; i < data_loader.m_output_data.size(); ++i)
    {
        data_loader.m_output_data[i] = data_loader.createDataPack(output_sample_count_per_dim, i);
    } // end for

    // allocate buffers for the data

    std::size_t single_size = sizeof(T);
    std::vector<std::uint64_t> input_buffer_sizes(input_dim);
    std::vector<std::uint64_t> output_buffer_sizes(output_dim);
    std::transform(input_count_per_dim, input_count_per_dim + input_dim, input_buffer_sizes.begin(),
                   [single_size](std::uint64_t n) -> std::uint64_t { return n * single_size; });
    std::transform(output_count_per_dim, output_count_per_dim + output_dim, output_buffer_sizes.begin(),
                   [single_size](std::uint64_t n) -> std::uint64_t { return n * single_size; });
    data_loader.allocate(input_buffer_sizes.data(), input_buffer_sizes.size(),
                         output_buffer_sizes.data(), output_buffer_sizes.size(),
                         allocate_output);
}

template <typename T>
inline void PartialDataLoaderHelper<T>::loadFromFile(PartialDataLoader &data_loader,
                                                     const std::string &filename,
                                                     std::size_t expected_input_dim,
                                                     const std::size_t *max_input_sample_count_per_dim,
                                                     const std::uint64_t *expected_input_count_per_dim,
                                                     std::size_t expected_output_dim,
                                                     const std::uint64_t *expected_output_count_per_dim)
{
    if (expected_input_dim <= 0)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid input dimensions: 'expected_input_dim' must be positive."));
    if (expected_output_dim <= 0)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid output dimensions: 'expected_output_dim' must be positive."));

    hebench::DataLoader::ExternalDataset<T> dataset = hebench::DataLoader::ExternalDatasetLoader<T>::loadFromCSV(filename, 0);
    std::size_t max_output_sample_count             = 1;

    // validate loaded data

    // inputs
    if (dataset.inputs.size() != expected_input_dim)
    {
        std::stringstream ss;
        ss << "Loaded input dimensions do not match the number of parameters for the operation. "
           << "Expected " << expected_input_dim << ", but " << dataset.inputs.size() << "loaded.";
        throw std::runtime_error(IL_LOG_MSG_CLASS(ss.str()));
    } // end if
    for (std::size_t input_dim_i = 0; input_dim_i < dataset.inputs.size(); ++input_dim_i)
    {
        std::vector<std::vector<T>> &input_component = dataset.inputs[input_dim_i];
        if (input_component.size() < max_input_sample_count_per_dim[input_dim_i])
        {
            std::stringstream ss;
            ss << "Insufficient data loaded for operation input parameter " << input_dim_i << ". "
               << "Expected " << max_input_sample_count_per_dim[input_dim_i] << " samples, but "
               << input_component.size() << " found.";
            throw std::runtime_error(IL_LOG_MSG_CLASS(ss.str()));
        } // end if
        if (input_component.size() > max_input_sample_count_per_dim[input_dim_i])
            // discard excess data
            input_component.resize(max_input_sample_count_per_dim[input_dim_i]);
        max_output_sample_count *= input_component.size(); // count the samples into the output
        // check each sample size
        for (std::size_t input_sample_i = 0; input_sample_i < input_component.size(); ++input_sample_i)
        {
            if (input_component[input_sample_i].size() != expected_input_count_per_dim[input_dim_i])
            {
                std::stringstream ss;
                ss << "Incorrect vector size loaded for input dimension " << input_dim_i << ", sample " << input_sample_i << ". "
                   << "Expected vector with " << expected_input_count_per_dim[input_dim_i] << " elements, but "
                   << input_component[input_sample_i].size() << " received.";
                throw std::runtime_error(IL_LOG_MSG_CLASS(ss.str()));
            } // end if
        } // end for
    } // end for

    // outputs
    if (!dataset.outputs.empty()
        && dataset.outputs.size() != expected_output_dim)
    {
        std::stringstream ss;
        ss << "Loaded output dimensions do not match the dimensions of the result for the operation. "
           << "Expected " << expected_output_dim << ", but " << dataset.outputs.size() << "loaded.";
        throw std::runtime_error(IL_LOG_MSG_CLASS(ss.str()));
    } // end if
    for (std::size_t output_dim_i = 0; output_dim_i < dataset.outputs.size(); ++output_dim_i)
    {
        std::vector<std::vector<T>> &output_component = dataset.outputs[output_dim_i];
        if (output_component.size() < max_output_sample_count)
        {
            // must have a ground truth for each combination of inputs
            std::stringstream ss;
            ss << "Insufficient ground-truth output samples loaded for output component " << output_dim_i << ". "
               << "Expected, at least, " << max_input_sample_count_per_dim << " samples, but "
               << output_component.size() << " received.";
            throw std::runtime_error(IL_LOG_MSG_CLASS(ss.str()));
        } // end if
        if (output_component.size() > max_output_sample_count)
            // discard excess data
            output_component.resize(max_output_sample_count);
        for (std::size_t output_sample_i = 0; output_sample_i < output_component.size(); ++output_sample_i)
        {
            if (output_component[output_sample_i].size() != expected_output_count_per_dim[output_dim_i])
            {
                std::stringstream ss;
                ss << "Incorrect vector size loaded for output dimension " << output_dim_i << ", sample " << output_sample_i << ". "
                   << "Expected vector with " << expected_output_count_per_dim[output_dim_i] << " elements, but "
                   << output_component[output_sample_i].size() << " received.";
                throw std::runtime_error(IL_LOG_MSG_CLASS(ss.str()));
            } // end if
        } // end for
    } // end for

    // initialize the data loader object

    std::vector<std::size_t> input_sample_count_per_dim(dataset.inputs.size());
    for (std::size_t i = 0; i < dataset.inputs.size(); ++i)
        input_sample_count_per_dim[i] = dataset.inputs[i].size();
    init(data_loader,
         dataset.inputs.size(), input_sample_count_per_dim.data(), expected_input_count_per_dim,
         expected_output_dim, expected_output_count_per_dim, !dataset.outputs.empty());

    assert(data_loader.getParameterCount() == dataset.inputs.size());
    assert(data_loader.getResultCount() == expected_output_dim);

    // copy loaded data into internal allocation

    for (std::size_t input_dim_i = 0; input_dim_i < dataset.inputs.size(); ++input_dim_i)
    {
        // input parameter
        const std::vector<std::vector<T>> &input_component = dataset.inputs[input_dim_i];
        hebench::APIBridge::DataPack &data_pack            = *data_loader.m_input_data[input_dim_i];
        assert(data_pack.param_position == input_dim_i
               && data_pack.buffer_count == input_component.size());
        for (std::size_t sample_i = 0; sample_i < data_pack.buffer_count; ++sample_i)
        {
            // parameter sample
            const std::vector<T> &input_component_sample       = input_component[sample_i];
            hebench::APIBridge::NativeDataBuffer &param_sample = data_pack.p_buffers[sample_i];
            assert(param_sample.p
                   && param_sample.size == input_component_sample.size() * sizeof(T));
            std::memcpy(param_sample.p, input_component_sample.data(), param_sample.size);
        } // end for
    } // end for

    for (std::size_t output_dim_i = 0; output_dim_i < dataset.outputs.size(); ++output_dim_i)
    {
        // output component
        const std::vector<std::vector<T>> &output_component = dataset.outputs[output_dim_i];
        hebench::APIBridge::DataPack &data_pack             = *data_loader.m_output_data[output_dim_i];
        assert(data_pack.param_position == output_dim_i
               && data_pack.buffer_count == output_component.size());
        for (std::size_t sample_i = 0; sample_i < data_pack.buffer_count; ++sample_i)
        {
            // parameter sample
            const std::vector<T> &output_component_sample                 = output_component[sample_i];
            hebench::APIBridge::NativeDataBuffer &result_component_sample = data_pack.p_buffers[sample_i];
            assert(result_component_sample.p
                   && result_component_sample.size == output_component_sample.size() * sizeof(T));
            std::memcpy(result_component_sample.p, output_component_sample.data(), result_component_sample.size);
        } // end for
    } // end for
}

//-------------------------
// class PartialDataLoader
//-------------------------

PartialDataLoader::PartialDataLoader() :
    m_data_type(hebench::APIBridge::DataType::Int32),
    m_b_is_output_allocated(false),
    m_b_initialized(false)
{
}

void PartialDataLoader::init(hebench::APIBridge::DataType data_type,
                             std::size_t input_dim,
                             const std::size_t *input_sample_count_per_dim,
                             const std::uint64_t *input_count_per_dim,
                             std::size_t output_dim,
                             const std::uint64_t *output_count_per_dim,
                             bool allocate_output)
{
    switch (data_type)
    {
    case hebench::APIBridge::DataType::Int32:
        PartialDataLoaderHelper<std::int32_t>::init(*this, input_dim, input_sample_count_per_dim, input_count_per_dim,
                                                    output_dim, output_count_per_dim,
                                                    allocate_output);
        break;

    case hebench::APIBridge::DataType::Int64:
        PartialDataLoaderHelper<std::int64_t>::init(*this, input_dim, input_sample_count_per_dim, input_count_per_dim,
                                                    output_dim, output_count_per_dim,
                                                    allocate_output);
        break;

    case hebench::APIBridge::DataType::Float32:
        PartialDataLoaderHelper<float>::init(*this, input_dim, input_sample_count_per_dim, input_count_per_dim,
                                             output_dim, output_count_per_dim,
                                             allocate_output);
        break;

    case hebench::APIBridge::DataType::Float64:
        PartialDataLoaderHelper<double>::init(*this, input_dim, input_sample_count_per_dim, input_count_per_dim,
                                              output_dim, output_count_per_dim,
                                              allocate_output);
        break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Unknown 'data_type'."));
        break;
    } // end switch

    m_data_type     = data_type;
    m_b_initialized = true;
}

void PartialDataLoader::init(const std::string &filename,
                             hebench::APIBridge::DataType data_type,
                             std::size_t expected_input_dim,
                             const std::size_t *max_input_sample_count_per_dim,
                             const std::uint64_t *expected_input_count_per_dim,
                             std::size_t expected_output_dim,
                             const std::uint64_t *expected_output_count_per_dim)
{
    switch (data_type)
    {
    case hebench::APIBridge::DataType::Int32:
        PartialDataLoaderHelper<std::int32_t>::loadFromFile(*this, filename,
                                                            expected_input_dim, max_input_sample_count_per_dim, expected_input_count_per_dim,
                                                            expected_output_dim, expected_output_count_per_dim);
        break;

    case hebench::APIBridge::DataType::Int64:
        PartialDataLoaderHelper<std::int64_t>::loadFromFile(*this, filename,
                                                            expected_input_dim, max_input_sample_count_per_dim, expected_input_count_per_dim,
                                                            expected_output_dim, expected_output_count_per_dim);
        break;

    case hebench::APIBridge::DataType::Float32:
        PartialDataLoaderHelper<float>::loadFromFile(*this, filename,
                                                     expected_input_dim, max_input_sample_count_per_dim, expected_input_count_per_dim,
                                                     expected_output_dim, expected_output_count_per_dim);
        break;

    case hebench::APIBridge::DataType::Float64:
        PartialDataLoaderHelper<double>::loadFromFile(*this, filename,
                                                      expected_input_dim, max_input_sample_count_per_dim, expected_input_count_per_dim,
                                                      expected_output_dim, expected_output_count_per_dim);
        break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Unknown 'data_type'."));
        break;
    } // end switch

    m_data_type     = data_type;
    m_b_initialized = true;
}

void PartialDataLoader::allocate(const std::uint64_t *input_buffer_sizes,
                                 std::size_t input_buffer_sizes_count,
                                 const std::uint64_t *output_buffer_sizes,
                                 std::size_t output_buffer_sizes_count,
                                 bool allocate_output)
{
    // allocate data for parameters and results

    if (!input_buffer_sizes)
    {
        std::stringstream ss;
        ss << "Invalid null `input_buffer_sizes`.";
        throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
    } // end if
    if (!output_buffer_sizes)
    {
        std::stringstream ss;
        ss << "Invalid null `output_buffer_sizes`.";
        throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
    } // end if

    m_b_is_output_allocated = allocate_output;

    // cache batch sizes
    std::vector<std::uint64_t> input_batch_sizes(m_input_data.size());
    for (std::size_t i = 0; i < m_input_data.size(); ++i)
        input_batch_sizes[i] = m_input_data[i]->buffer_count;
    std::vector<std::uint64_t> output_batch_sizes(m_output_data.size());
    for (std::size_t i = 0; i < m_output_data.size(); ++i)
        output_batch_sizes[i] = m_output_data[i]->buffer_count;

    if (input_buffer_sizes_count < input_batch_sizes.size())
    {
        std::stringstream ss;
        ss << "Invalid number of input buffers `input_buffer_sizes_count`. Expected, at least "
           << input_batch_sizes.size() << ", but " << input_buffer_sizes_count << " received.";
        throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
    } // end if
    if (output_buffer_sizes_count < output_batch_sizes.size())
    {
        std::stringstream ss;
        ss << "Invalid number of output buffers `output_buffer_sizes_count`. Expected, at least "
           << output_batch_sizes.size() << ", but " << output_buffer_sizes_count << " received.";
        throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
    } // end if

    // compute total space required for the data buffers
    std::uint64_t total_raw_size = 0;
    std::uint64_t output_start;
    for (std::size_t i = 0; i < input_batch_sizes.size(); ++i)
        total_raw_size += input_buffer_sizes[i] * input_batch_sizes[i];
    output_start = total_raw_size;
    for (std::size_t i = 0; i < output_batch_sizes.size(); ++i)
        total_raw_size += output_buffer_sizes[i] * (allocate_output ? output_batch_sizes[i] : 1);

    // allocate space for the data buffers
    m_raw_buffer.resize(total_raw_size, 0);

    // m_raw_buffer has a chunk of memory allocated enough to hold the all the data and
    // now, we have to point the NativeDataBuffers for each sample to the right locations:

    // param[0, 0] param[0, 1] param[0, 2] param[1, 0]     param[1, 1]     param[1, 2] ...
    //   v          v           v           v               v               v
    // [ ***********************************************************************************... ]

    // initialize the buffers

    // point parameter start buffers to correct start location
    std::vector<std::uint8_t *> input_buffers(input_batch_sizes.size(), m_raw_buffer.data());
    for (std::uint64_t i = 1; i < input_batch_sizes.size(); ++i)
        input_buffers[i] = input_buffers[i - 1] + input_buffer_sizes[i - 1] * input_batch_sizes[i - 1];
    if (!input_buffers.empty())
    {
        // make sure we are pointing to the right location in the master buffer
        assert(input_buffers.front() == m_raw_buffer.data());
    } // end if
    std::vector<std::uint8_t *> output_buffers;
    if (allocate_output)
    {
        output_buffers.resize(output_batch_sizes.size(), m_raw_buffer.data() + output_start);
        for (std::uint64_t i = 1; i < output_batch_sizes.size(); ++i)
            output_buffers[i] = output_buffers[i - 1] + output_buffer_sizes[i - 1] * output_batch_sizes[i - 1];
        if (!output_buffers.empty())
        {
            // make sure we are pointing to the right location in the master buffer
            assert(output_buffers.front() == m_raw_buffer.data() + output_start);
        } //end if
    } // end if

    // point the NativeDataBuffers for each data sample to the correct memory location

    // input
    for (std::size_t param_i = 0; param_i < m_input_data.size(); ++param_i)
    {
        //#pragma omp parallel for
        for (std::uint64_t i = 0; i < input_batch_sizes[param_i]; ++i)
        {
            // point to the start of the data in the raw data buffer
            m_input_data[param_i]->p_buffers[i].p    = input_buffers[param_i] + i * input_buffer_sizes[param_i];
            m_input_data[param_i]->p_buffers[i].size = input_buffer_sizes[param_i];
            m_input_data[param_i]->p_buffers[i].tag  = 0;
        } // end for
    } // end for
    // output
    for (std::size_t output_i = 0; output_i < m_output_data.size(); ++output_i)
    {
        //#pragma omp parallel for
        for (std::uint64_t i = 0; i < output_batch_sizes[output_i]; ++i)
        {
            // point to the start of the data in the raw data buffer
            m_output_data[output_i]->p_buffers[i].p = allocate_output ?
                                                          output_buffers[output_i] + i * output_buffer_sizes[output_i] :
                                                          nullptr;
            m_output_data[output_i]->p_buffers[i].size = output_buffer_sizes[output_i];
            m_output_data[output_i]->p_buffers[i].tag  = 0;
        } // end for
    } // end for

    // all data has been allocated and pointed to at this point
}

std::vector<std::shared_ptr<hebench::APIBridge::DataPack>> PartialDataLoader::getResultTempDataPacks(std::uint64_t result_index) const
{
    if (!m_b_initialized)
        throw std::logic_error(IL_LOG_MSG_CLASS("Not initialized."));

    std::vector<std::shared_ptr<hebench::APIBridge::DataPack>> retval(m_output_data.size());

    for (std::size_t result_component_i = 0; result_component_i < m_output_data.size(); ++result_component_i)
    {
        if (!m_output_data[result_component_i]
            || !m_output_data[result_component_i]->p_buffers)
            throw std::logic_error(IL_LOG_MSG_CLASS("Description for output component " + std::to_string(result_component_i) + " is not initialized."));
        if (result_index >= m_output_data[result_component_i]->buffer_count)
        {
            std::stringstream ss;
            ss << "Out of range `result_index`."
               << " Expected value less than " << m_output_data[result_component_i]->buffer_count << ", but "
               << result_index << " received.";
            throw std::out_of_range(IL_LOG_MSG_CLASS(ss.str()));
        } // end if
        retval[result_component_i] =
            std::shared_ptr<hebench::APIBridge::DataPack>(new hebench::APIBridge::DataPack(),
                                                          [](hebench::APIBridge::DataPack *p) {
                                                              if (p)
                                                              {
                                                                  if (p->p_buffers)
                                                                  {
                                                                      for (std::uint64_t buffer_i = 0; buffer_i < p->buffer_count; ++buffer_i)
                                                                      {
                                                                          hebench::APIBridge::NativeDataBuffer &buffer = p->p_buffers[buffer_i];
                                                                          if (buffer.p)
                                                                              delete[] reinterpret_cast<std::int8_t *>(buffer.p);
                                                                      } // end for
                                                                      delete[] p->p_buffers;
                                                                  } // end if
                                                                  delete p;
                                                              } // end if
                                                          });
        retval[result_component_i]->param_position = result_component_i;
        retval[result_component_i]->buffer_count   = 1;
        retval[result_component_i]->p_buffers      = new hebench::APIBridge::NativeDataBuffer[retval[result_component_i]->buffer_count];
        retval[result_component_i]->p_buffers[0]   = m_output_data[result_component_i]->p_buffers[result_index];
        retval[result_component_i]->p_buffers[0].p = new std::int8_t[retval[result_component_i]->p_buffers[0].size];
    } // end for

    return retval;
}

const hebench::APIBridge::DataPack &PartialDataLoader::getParameterData(std::uint64_t param_position) const
{
    if (!m_b_initialized)
        throw std::logic_error(IL_LOG_MSG_CLASS("Not initialized."));

    if (!m_input_data.at(param_position))
        throw std::runtime_error(IL_LOG_MSG_CLASS("Invalid null element accessed at 'param_position'."));

    return *m_input_data.at(param_position);
}

const hebench::APIBridge::DataPack &PartialDataLoader::getResultData(std::uint64_t param_position) const
{
    if (!m_b_initialized)
        throw std::logic_error(IL_LOG_MSG_CLASS("Not initialized."));

    if (!m_output_data.at(param_position))
        throw std::runtime_error(IL_LOG_MSG_CLASS("Invalid null element accessed at 'param_position'."));

    return *m_output_data.at(param_position);
}

IDataLoader::ResultDataPtr PartialDataLoader::getResultFor(const std::uint64_t *param_data_pack_indices)
{
    if (!m_b_initialized)
        throw std::logic_error(IL_LOG_MSG_CLASS("Not initialized."));

    ResultDataPtr p_retval                                            = ResultDataPtr(new ResultData());
    std::vector<const hebench::APIBridge::NativeDataBuffer *> &retval = p_retval->result;
    std::uint64_t r_i                                                 = getResultIndex(param_data_pack_indices);
    p_retval->sample_index                                            = r_i;

    retval.resize(getResultCount());
    for (std::size_t result_component_i = 0; result_component_i < retval.size(); ++result_component_i)
    {
        const hebench::APIBridge::DataPack &result_component = getResultData(result_component_i);
        assert((result_component.buffer_count == 0 || result_component.p_buffers)
               && result_component.param_position == result_component_i);
        if (r_i >= result_component.buffer_count)
        {
            std::stringstream ss;
            ss << "Unexpected error! Result sample " << r_i << " for result component " << result_component_i << " not found.";
            throw std::logic_error(IL_LOG_MSG_CLASS(ss.str()));
        } // end if
        retval[result_component_i] = result_component.p_buffers + r_i;
    } // end for

    return p_retval;
}

std::uint64_t PartialDataLoader::getResultIndex(const std::uint64_t *param_data_pack_indices) const
{
    if (!m_b_initialized)
        throw std::logic_error(IL_LOG_MSG_CLASS("Not initialized."));

    if (!param_data_pack_indices)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null argument 'param_data_pack_indices'."));

    std::uint64_t retval = getParameterCount() > 0 ?
                               param_data_pack_indices[0] :
                               0;

    for (std::size_t param_i = 1; param_i < getParameterCount(); ++param_i)
    {
        assert(getParameterData(param_i).param_position == param_i);
        if (param_data_pack_indices[param_i] >= getParameterData(param_i).buffer_count)
        {
            std::stringstream ss;
            ss << "Index out of range: 'param_data_pack_indices['" << param_i << "] == " << param_data_pack_indices[param_i] << ". "
               << "Expected value less than " << getParameterData(param_i).buffer_count << ".";
            throw std::out_of_range(IL_LOG_MSG_CLASS(ss.str()));
        } // end if
        retval = param_data_pack_indices[param_i] + getParameterData(param_i).buffer_count * retval;
    } // end for

    return retval;
}

} // namespace TestHarness
} // namespace hebench
