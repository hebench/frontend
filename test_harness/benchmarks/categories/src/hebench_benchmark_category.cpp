
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <cassert>
#include <sstream>

#include "hebench/api_bridge/api.h"

#include "../include/hebench_benchmark_category.h"
#include "include/hebench_math_utils.h"
#include "include/hebench_utilities.h"

namespace hebench {
namespace TestHarness {

//-------------------
// struct RAIIHandle
//-------------------

PartialBenchmarkCategory::RAIIHandle::RAIIHandle()
{
    handle.p    = nullptr;
    handle.size = 0;
    handle.tag  = 0;
}

PartialBenchmarkCategory::RAIIHandle::RAIIHandle(RAIIHandle &&rhs) noexcept :
    handle(rhs.handle)
{
    rhs.detach();
}

PartialBenchmarkCategory::RAIIHandle &PartialBenchmarkCategory::RAIIHandle::operator=(RAIIHandle &&rhs) noexcept
{
    if (&rhs != this)
    {
        this->handle = rhs.handle;
        rhs.detach();
    } // end if
    return *this;
}

PartialBenchmarkCategory::RAIIHandle &PartialBenchmarkCategory::RAIIHandle::operator=(const hebench::APIBridge::Handle &rhs)
{
    destroy();
    this->handle = rhs;
    return *this;
}

PartialBenchmarkCategory::RAIIHandle::~RAIIHandle()
{
    destroy();
}

bool PartialBenchmarkCategory::RAIIHandle::isEmpty(const hebench::APIBridge::Handle &h) noexcept
{
    return (!h.p && h.size == 0 && h.tag == 0);
}

void PartialBenchmarkCategory::RAIIHandle::detach()
{
    handle.p    = nullptr;
    handle.size = 0;
    handle.tag  = 0;
}

hebench::APIBridge::ErrorCode PartialBenchmarkCategory::RAIIHandle::destroy()
{
    hebench::APIBridge::ErrorCode retval = HEBENCH_ECODE_SUCCESS;
    if (!isEmpty(handle))
    {
        retval = hebench::APIBridge::destroyHandle(handle);
        detach();
    } // end if
    return retval;
}

//--------------------------------
// class PartialBenchmarkCategory
//--------------------------------

PartialBenchmarkCategory::PartialBenchmarkCategory(std::shared_ptr<Engine> p_engine,
                                                   const IBenchmarkDescriptor::DescriptionToken &description_token) :
    PartialBenchmark(p_engine, description_token)
{
}

PartialBenchmarkCategory::~PartialBenchmarkCategory()
{
}

bool PartialBenchmarkCategory::validateResult(IDataLoader::Ptr dataset,
                                              const std::uint64_t *param_data_pack_indices,
                                              const std::vector<hebench::APIBridge::NativeDataBuffer *> &outputs,
                                              hebench::APIBridge::DataType data_type) const
{
    static constexpr const std::size_t MaxErrorPrint = 10;
    bool retval                                      = true;
    std::vector<std::uint64_t> is_valid;

    // extract the pointers to the actual results

    if (outputs.size() != dataset->getResultCount())
    {
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid number of outputs: 'outputs'."));
    }

    std::vector<const hebench::APIBridge::NativeDataBuffer *> truths =
        dataset->getResultFor(param_data_pack_indices);

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
                    throw std::invalid_argument(IL_LOG_MSG_CLASS("Unexpected null output component in: 'outputs[" + std::to_string(index) + "]'."));
                }
            }
            catch (const std::out_of_range &out_of_range)
            {
                throw std::invalid_argument(IL_LOG_MSG_CLASS("Unexpected out of range index output component in: 'outputs[" + std::to_string(index) + "]'."));
            }

            if (outputs.at(index)->size < truths.at(index)->size)
            {
                throw std::invalid_argument(IL_LOG_MSG_CLASS("Buffer in outputs is not large enough to contain the expected output: 'outputs[" + std::to_string(index) + "]'."));
            }

            std::uint64_t count = truths.at(index)->size / IDataLoader::sizeOf(data_type);
            void *p_truth       = truths.at(index)->p;
            void *p_output      = outputs.at(index)->p; // single output

            // validate the results
            switch (data_type)
            {
            case hebench::APIBridge::DataType::Int32:
                is_valid = hebench::Utilities::Math::almostEqual(reinterpret_cast<const std::int32_t *>(p_truth),
                                                                 reinterpret_cast<const std::int32_t *>(p_output),
                                                                 count,
                                                                 0.01);
                break;

            case hebench::APIBridge::DataType::Int64:
                is_valid = hebench::Utilities::Math::almostEqual(reinterpret_cast<const std::int64_t *>(p_truth),
                                                                 reinterpret_cast<const std::int64_t *>(p_output),
                                                                 count,
                                                                 0.01);
                break;

            case hebench::APIBridge::DataType::Float32:
                is_valid = hebench::Utilities::Math::almostEqual(reinterpret_cast<const float *>(p_truth),
                                                                 reinterpret_cast<const float *>(p_output),
                                                                 count,
                                                                 0.01);
                break;

            case hebench::APIBridge::DataType::Float64:
                is_valid = hebench::Utilities::Math::almostEqual(reinterpret_cast<const double *>(p_truth),
                                                                 reinterpret_cast<const double *>(p_output),
                                                                 count,
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
            ss << "Result component " << index << "; elements not within 1% of each other, " << is_valid.size() << std::endl
               << "Failed indices, ";
            for (std::size_t i = 0; i < is_valid.size() && i < MaxErrorPrint; ++i)
            {
                ss << is_valid[i];
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

void PartialBenchmarkCategory::logResult(std::ostream &os,
                                         IDataLoader::Ptr dataset,
                                         const std::uint64_t *param_data_pack_indices,
                                         const std::vector<hebench::APIBridge::NativeDataBuffer *> &outputs,
                                         hebench::APIBridge::DataType data_type) const
{
    std::stringstream ss;

    os << "Number of parameters, " << dataset->getParameterCount() << std::endl
       << "Number of result components (expected), " << dataset->getResultCount() << std::endl
       << "Number of result components (received), " << outputs.size() << std::endl
       << std::endl;

    std::vector<const hebench::APIBridge::NativeDataBuffer *> truths =
        dataset->getResultFor(param_data_pack_indices);

    // param_data_pack_indices already validated by previous call

    ss << ", Parameters, ";

    os << "Parameter index, size" << std::endl;
    for (std::size_t param_i = 0; param_i < dataset->getParameterCount(); ++param_i)
    {
        assert(dataset->getParameterData(param_i).param_position == param_i);
        const hebench::APIBridge::NativeDataBuffer &arg = dataset->getParameterData(param_i).p_buffers[param_data_pack_indices[param_i]];
        os << param_i << ", " << arg.size / IDataLoader::sizeOf(data_type) << std::endl;

        if (param_i > 0)
            ss << ", ";
    } // end for

    ss << "Ground truth, ";

    os << std::endl
       << "Ground truth index, size" << std::endl;
    for (std::size_t result_component_i = 0; result_component_i < truths.size(); ++result_component_i)
    {
        assert(truths[result_component_i]);
        os << result_component_i << ", " << truths[result_component_i]->size / IDataLoader::sizeOf(data_type) << std::endl;

        if (result_component_i > 0)
            ss << ", ";
    } // end for

    ss << "Output";

    os << std::endl
       << "Output index, size" << std::endl;
    for (std::size_t result_component_i = 0; result_component_i < outputs.size(); ++result_component_i)
    {
        assert(outputs[result_component_i]);
        os << result_component_i << ", " << outputs[result_component_i]->size / IDataLoader::sizeOf(data_type) << std::endl;

        if (result_component_i + 1 < outputs.size())
            ss << ", ";
    } // end for

    // table header
    os << std::endl
       << ss.str() << std::endl
       << "Index, ";

    // prepare arguments to print out in columns
    std::vector<const hebench::APIBridge::NativeDataBuffer *> all_values;

    // parameters
    for (std::size_t param_i = 0; param_i < dataset->getParameterCount(); ++param_i)
    {
        all_values.push_back(&dataset->getParameterData(param_i).p_buffers[param_data_pack_indices[param_i]]);
        os << param_i << ", ";
    } // end for
    // ground truth
    for (std::size_t result_component_i = 0; result_component_i < truths.size(); ++result_component_i)
    {
        all_values.push_back(truths[result_component_i]);
        os << result_component_i << ", ";
    } // end for
    // received output
    for (std::size_t result_component_i = 0; result_component_i < outputs.size(); ++result_component_i)
    {
        if (result_component_i > 0)
            os << ", ";
        all_values.push_back(outputs[result_component_i]);
        os << result_component_i;
    } // end for

    os << std::endl;
    hebench::Utilities::printArraysAsColumns(os, all_values.data(), all_values.size(),
                                             data_type, true, ", ");
}

} // namespace TestHarness
} // namespace hebench
