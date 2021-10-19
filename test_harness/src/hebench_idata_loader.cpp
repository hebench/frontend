
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <functional>
#include <sstream>
#include <stdexcept>

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

IDataLoader::unique_ptr_custom_deleter<hebench::APIBridge::PackedData>
IDataLoader::createPackedData(std::uint64_t data_pack_count)
{
    unique_ptr_custom_deleter<hebench::APIBridge::PackedData> retval;
    hebench::APIBridge::DataPack *p_data_packs = nullptr;

    try
    {
        p_data_packs = new hebench::APIBridge::DataPack[data_pack_count];
        retval       = unique_ptr_custom_deleter<hebench::APIBridge::PackedData>(
            new hebench::APIBridge::PackedData(),
            [](hebench::APIBridge::PackedData *p) {
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

//-------------------------
// class PartialDataLoader
//-------------------------

void PartialDataLoader::init(std::size_t input_dim,
                             const std::size_t *input_count_per_dim,
                             std::size_t output_dim)
{
    if (input_dim <= 0)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid input dimensions: 'input_dim' must be positive."));
    if (output_dim <= 0)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid output dimensions: 'output_dim' must be positive."));

    m_input_data.resize(input_dim);
    m_output_data.resize(output_dim);

    std::size_t output_count_per_dim = 1;
    for (std::size_t i = 0; i < m_input_data.size(); ++i)
    {
        if (input_count_per_dim[i] <= 0)
        {
            std::stringstream ss;
            ss << "Invalid batch size for dimension " << i << ": 'input_count_per_dim[" << i << "]' must be positive.";
            throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
        } // end if

        output_count_per_dim *= input_count_per_dim[i];
        m_input_data[i] = createDataPack(input_count_per_dim[i], i);
    } // end for

    for (std::size_t i = 0; i < m_output_data.size(); ++i)
    {
        m_output_data[i] = createDataPack(output_count_per_dim, i);
    } // end for
}

void PartialDataLoader::allocate(const std::uint64_t *input_buffer_sizes,
                                 std::size_t input_buffer_sizes_count,
                                 const std::uint64_t *output_buffer_sizes,
                                 std::size_t output_buffer_sizes_count,
                                 bool allocate_output)
{
    // allocate data for parameters and results

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
        ss << "Invalid number of input buffers 'input_buffer_sizes_count'. Expected, at least "
           << input_batch_sizes.size() << ", but " << input_buffer_sizes_count << " received.";
        throw std::invalid_argument(IL_LOG_MSG_CLASS(ss.str()));
    } // end if
    if (output_buffer_sizes_count < output_batch_sizes.size())
    {
        std::stringstream ss;
        ss << "Invalid number of output buffers 'output_buffer_sizes_count'. Expected, at least "
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
        total_raw_size += output_buffer_sizes[i] * output_batch_sizes[i];
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
        input_buffers[i] += i * input_buffer_sizes[i - 1] * input_batch_sizes[i - 1];
    if (!input_buffers.empty())
        // make sure we are pointing to the right location in the master buffer
        assert(input_buffers.front() == m_raw_buffer.data());
    std::vector<std::uint8_t *> output_buffers;
    if (allocate_output)
    {
        output_buffers.resize(output_batch_sizes.size(), m_raw_buffer.data() + output_start);
        for (std::uint64_t i = 1; i < output_batch_sizes.size(); ++i)
            output_buffers[i] += i * output_buffer_sizes[i - 1] * output_batch_sizes[i - 1];
        if (!output_buffers.empty())
            // make sure we are pointing to the right location in the master buffer
            assert(output_buffers.front() == m_raw_buffer.data() + output_start);
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
            m_output_data[output_i]->p_buffers[i].p    = allocate_output ?
                                                             output_buffers[output_i] + i * output_buffer_sizes[output_i] :
                                                             nullptr;
            m_output_data[output_i]->p_buffers[i].size = output_buffer_sizes[output_i];
            m_output_data[output_i]->p_buffers[i].tag  = 0;
        } // end for
    } // end for

    // all data has been allocated and pointed to at this point
}

const hebench::APIBridge::DataPack &PartialDataLoader::getParameterData(std::uint64_t param_position) const
{
    if (!m_input_data.at(param_position))
        throw std::runtime_error(IL_LOG_MSG_CLASS("Invalid null element accessed at 'param_position'."));

    return *m_input_data.at(param_position);
}

const hebench::APIBridge::DataPack &PartialDataLoader::getResultData(std::uint64_t param_position) const
{
    if (!m_output_data.at(param_position))
        throw std::runtime_error(IL_LOG_MSG_CLASS("Invalid null element accessed at 'param_position'."));

    return *m_output_data.at(param_position);
}

std::vector<const hebench::APIBridge::NativeDataBuffer *>
PartialDataLoader::getResultFor(const std::uint64_t *param_data_pack_indices)
{
    std::vector<const hebench::APIBridge::NativeDataBuffer *> retval;
    std::uint64_t r_i = getResultIndex(param_data_pack_indices);

    retval.resize(getResultCount());
    for (std::size_t result_component_i = 0; result_component_i < retval.size(); ++result_component_i)
    {
        assert(getResultData(result_component_i).param_position == result_component_i);
        retval[result_component_i] = getResultData(result_component_i).p_buffers + r_i;
        assert(retval[result_component_i]);
    } // end for

    return retval;
}

std::uint64_t PartialDataLoader::getResultIndex(const std::uint64_t *param_data_pack_indices)
{
    if (!param_data_pack_indices)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null argument 'param_data_pack_indices'."));

    std::uint64_t retval = getParameterCount() > 0 ?
                               param_data_pack_indices[0] :
                               0;

    for (std::size_t param_i = 1; param_i < getParameterCount(); ++param_i)
    {
        assert(getParameterData(param_i).param_position == param_i);
        if (param_data_pack_indices[param_i] >= getParameterData(param_i).buffer_count)
            throw std::out_of_range(IL_LOG_MSG_CLASS("Index out of range: 'param_data_pack_indices['" + std::to_string(param_i) + "] == " + std::to_string(param_data_pack_indices[param_i]) + ". Expected value less than " + std::to_string(getParameterData(param_i).buffer_count) + "."));
        retval = param_data_pack_indices[param_i] + getParameterData(param_i).buffer_count * retval;
    } // end for

    return retval;
}

} // namespace TestHarness
} // namespace hebench
