
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include <array>
#include <cassert>
#include <cmath>
#include <iterator>
#include <random>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

#include "../include/hebench_logreg.h"
#include "modules/general/include/hebench_math_utils.h"

namespace hebench {
namespace TestHarness {
namespace LogisticRegression {

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
    assert(OpParameterCount == 3);
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
    if (bench_desc.workload == hebench::APIBridge::Workload::LogisticRegression
        || bench_desc.workload == hebench::APIBridge::Workload::LogisticRegression_PolyD3
        || bench_desc.workload == hebench::APIBridge::Workload::LogisticRegression_PolyD5
        || bench_desc.workload == hebench::APIBridge::Workload::LogisticRegression_PolyD7)
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
    std::stringstream ss;

    output.concrete_descriptor = backend_desc.descriptor;
    if (getForceConfigValues())
    {
        output.concrete_descriptor.cat_params.min_test_time_ms =
            config.default_min_test_time_ms == 0 ?
                backend_desc.descriptor.cat_params.min_test_time_ms :
                config.default_min_test_time_ms;
    } // end if
    else
    {
        output.concrete_descriptor.cat_params.min_test_time_ms =
            backend_desc.descriptor.cat_params.min_test_time_ms != 0 ?
                backend_desc.descriptor.cat_params.min_test_time_ms :
                config.default_min_test_time_ms;
    } // end else

    // workload name

    std::uint64_t vector_size = fetchVectorSize(config.w_params);
    ss << BaseWorkloadName;
    switch (backend_desc.descriptor.workload)
    {
    case hebench::APIBridge::Workload::LogisticRegression_PolyD3:
        ss << " PolyD3";
        break;
    case hebench::APIBridge::Workload::LogisticRegression_PolyD5:
        ss << " PolyD5";
        break;
    case hebench::APIBridge::Workload::LogisticRegression_PolyD7:
        ss << " PolyD7";
        break;
    default:
        // standard sigmoid
        break;
    } // end switch

    output.workload_base_name = ss.str();
    ss << " " << vector_size << " features";
    output.workload_name          = ss.str();
    output.operation_params_count = BenchmarkDescriptorCategory::OpParameterCount;
}

//---------------------------
// class DataGeneratorHelper
//---------------------------

/**
 * @brief Static helper class to generate data for all supported data types.
 */
class DataGeneratorHelper : public hebench::TestHarness::DataGeneratorHelper
{
private:
    IL_DECLARE_CLASS_NAME(LogisticRegression::DataGeneratorHelper)

public:
    static void logisticRegressionInference(hebench::APIBridge::DataType data_type,
                                            DataLoader::PolynomialDegree poly_deg,
                                            void *result, const void *w, const void *b, const void *input,
                                            std::uint64_t feature_count);

protected:
    DataGeneratorHelper() = default;

private:
    template <class T, class Container> // T must always be arithmetic type, Container elements must be of type T
    static double evaluatePolynomial(T x, const Container &coeff);

    template <unsigned int degree> // degree must be only 0, 3, 5, or 7
    static double sigmoid(double x);

    template <class T> // T must always be floating point
    static void logisticRegressionInference(DataLoader::PolynomialDegree poly_deg,
                                            T &result, const T *p_w, const T &b, const T *p_input,
                                            std::uint64_t feature_count);
};

template <class T, class Container>
inline double DataGeneratorHelper::evaluatePolynomial(T x, const Container &coeff)
{
    // Horner's method follows:
    //    a_n * x^n + a_n-1 * x^(n-1) +... + a_1 * x + a_0
    // == (...(((a_n * x + a_n-1) * x + a_n-2) * x ... + a_1) * x + a_0
    auto it  = std::rbegin(coeff);
    T retval = *it;
    for (++it; it != std::rend(coeff); ++it)
        retval = retval * x + *it;

    return retval;
}

template <>
inline double DataGeneratorHelper::sigmoid<0>(double x)
{
    return 1.0 / (1.0 + std::exp(-x));
}

template <>
inline double DataGeneratorHelper::sigmoid<3>(double x)
{
    // f3(x) = 0.5 + 1.20096(x/8) - 0.81562(x/8)^3
    static const std::array<double, 4> poly = { 0.5, 0.15012, 0.0, -0.0015930078125 };
    return evaluatePolynomial(x, poly);
}

template <>
inline double DataGeneratorHelper::sigmoid<5>(double x)
{
    // f5(x) = 0.5 + 1.53048(x/8) - 2.3533056(x/8)^3 + 1.3511295(x/8)^5
    static const std::array<double, 6> poly = { 0.5, 0.19131, 0.0, -0.0045963, 0.0, 0.0000412332000732421875 };
    return evaluatePolynomial(x, poly);
}

template <>
inline double DataGeneratorHelper::sigmoid<7>(double x)
{
    // f7(x) = 0.5 + 1.73496(x/8) - 4.19407(x/8)^3 + 5.43402(x/8)^5 - 2.50739(x/8)^7
    static const std::array<double, 8> poly = { 0.5, 0.21687, 0.0, -0.00819154296875, 0.0, 0.0001658331298828125, 0.0, -0.00000119561672210693359375 };
    return evaluatePolynomial(x, poly);
}

template <class T>
inline void DataGeneratorHelper::logisticRegressionInference(DataLoader::PolynomialDegree poly_deg,
                                                             T &result, const T *p_w, const T &b, const T *p_input,
                                                             std::uint64_t feature_count)
{
    if (!p_w)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null 'p_w'."));
    if (!p_input)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null 'p_input'."));

    T linear_regression = std::inner_product(p_w, p_w + feature_count, p_input, static_cast<T>(0))
                          + b;
    switch (poly_deg)
    {
    case DataLoader::PolynomialDegree::PD3:
        result = static_cast<T>(sigmoid<3>(static_cast<double>(linear_regression)));
        break;

    case DataLoader::PolynomialDegree::PD5:
        result = static_cast<T>(sigmoid<5>(static_cast<double>(linear_regression)));
        break;

    case DataLoader::PolynomialDegree::PD7:
        result = static_cast<T>(sigmoid<7>(static_cast<double>(linear_regression)));
        break;

    default:
        result = static_cast<T>(sigmoid<0>(static_cast<double>(linear_regression)));
        break;
    } // end switch
}

void DataGeneratorHelper::logisticRegressionInference(hebench::APIBridge::DataType data_type,
                                                      DataLoader::PolynomialDegree poly_deg,
                                                      void *p_result, const void *p_w, const void *p_bias, const void *p_input,
                                                      std::uint64_t feature_count)
{
    if (!p_result)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null 'p_result'."));
    if (!p_bias)
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Invalid null 'p_bias'."));

    switch (data_type)
    {
    case hebench::APIBridge::DataType::Float32:
        logisticRegressionInference<float>(poly_deg,
                                           *reinterpret_cast<float *>(p_result),
                                           reinterpret_cast<const float *>(p_w),
                                           *reinterpret_cast<const float *>(p_bias),
                                           reinterpret_cast<const float *>(p_input),
                                           feature_count);
        break;

    case hebench::APIBridge::DataType::Float64:
        logisticRegressionInference<double>(poly_deg,
                                            *reinterpret_cast<double *>(p_result),
                                            reinterpret_cast<const double *>(p_w),
                                            *reinterpret_cast<const double *>(p_bias),
                                            reinterpret_cast<const double *>(p_input),
                                            feature_count);
        break;

    default:
        throw std::invalid_argument(IL_LOG_MSG_CLASS("Data type not supported."));
        break;
    } // end switch
}

//---------------------
// class DataGenerator
//---------------------

DataLoader::Ptr DataLoader::create(PolynomialDegree polynomial_degree,
                                   std::uint64_t vector_size,
                                   std::uint64_t batch_size_input,
                                   hebench::APIBridge::DataType data_type)
{
    DataLoader::Ptr retval = DataLoader::Ptr(new DataLoader());
    retval->init(polynomial_degree, vector_size, batch_size_input, data_type);
    return retval;
}

DataLoader::Ptr DataLoader::create(PolynomialDegree polynomial_degree,
                                   std::uint64_t vector_size,
                                   std::uint64_t batch_size_input,
                                   hebench::APIBridge::DataType data_type,
                                   const std::string &dataset_filename)
{
    DataLoader::Ptr retval = DataLoader::Ptr(new DataLoader());
    retval->init(polynomial_degree, vector_size, batch_size_input, data_type, dataset_filename);
    return retval;
}

DataLoader::DataLoader() :
    m_polynomial_degree(PolynomialDegree::None),
    m_vector_size(0)
{
}

void DataLoader::init(PolynomialDegree polynomial_degree,
                      std::uint64_t vector_size,
                      std::uint64_t batch_size_input,
                      hebench::APIBridge::DataType data_type)
{
    // Load/generate and initialize the data for logistic regression

    assert(InputDim0 + OutputDim0 >= 4);

    // number of samples in each input parameter and output
    std::size_t batch_sizes[InputDim0 + OutputDim0] = {
        1, // W
        1, // b
        batch_size_input, // X
        batch_size_input // result
    };
    m_polynomial_degree = polynomial_degree;
    m_vector_size       = vector_size;

    // compute buffer size in bytes for each vector
    std::uint64_t vector_sample_sizes[InputDim0 + OutputDim0] = {
        vector_size, // W
        1, // b
        vector_size, // X
        1 // result
    };

    // allocate memory for each vector buffer
    DataLoaderCompute::init(data_type,
                            InputDim0, // number of input components
                            batch_sizes, // number of samples per input component
                            vector_sample_sizes, // number of elements in each vector of the input
                            OutputDim0, // number of output components
                            vector_sample_sizes + InputDim0, // number of elements in each vector of the output
                            true); // allocate memory for results?

    // at this point all NativeDataBuffers have been allocated and pointed to the correct locations

    // fill up each vector data

    // input
    for (std::size_t vector_i = 0; vector_i < InputDim0; ++vector_i)
    {
        for (std::uint64_t i = 0; i < batch_sizes[vector_i]; ++i)
        {
            // generate the data
            DataGeneratorHelper::generateRandomVectorN(data_type,
                                                       getParameterData(vector_i).p_buffers[i].p,
                                                       getParameterData(vector_i).p_buffers[i].size / PartialDataLoader::sizeOf(data_type),
                                                       0.0, 1.0);
        } // end for
    } // end for

    // output
    //#pragma omp parallel for
    for (std::uint64_t input_i = 0; input_i < batch_sizes[2]; ++input_i)
    {
        // find the index for the result buffer based on the input indices
        std::uint64_t ppi[] = { 0, 0, input_i };
        std::uint64_t r_i   = getResultIndex(ppi);

        // generate the data
        DataGeneratorHelper::logisticRegressionInference(data_type, polynomial_degree,
                                                         getResultData(0).p_buffers[r_i].p, // result
                                                         getParameterData(Index_W).p_buffers[0].p, // W
                                                         getParameterData(Index_b).p_buffers[0].p, // b
                                                         getParameterData(Index_X).p_buffers[input_i].p, // X
                                                         vector_size);
    } // end for

    // all data has been generated at this point
}

void DataLoader::init(PolynomialDegree polynomial_degree,
                      std::uint64_t expected_vector_size,
                      std::uint64_t max_batch_size_input,
                      hebench::APIBridge::DataType data_type,
                      const std::string &dataset_filename)
{
    // Load/generate and initialize the data for logistic regression

    assert(InputDim0 + OutputDim0 >= 4);

    // number of samples in each input parameter and output
    std::size_t batch_sizes[InputDim0 + OutputDim0] = {
        1, // W
        1, // b
        max_batch_size_input, // X
        max_batch_size_input // result
    };
    m_polynomial_degree = polynomial_degree;
    m_vector_size       = expected_vector_size;

    // compute buffer size in bytes for each vector
    std::uint64_t vector_sample_sizes[InputDim0 + OutputDim0] = {
        expected_vector_size, // W
        1, // b
        expected_vector_size, // X
        1 // result
    };

    // allocate memory for each vector buffer
    DataLoaderCompute::init(dataset_filename, data_type,
                            InputDim0, // number of input components
                            batch_sizes, // number of samples per input component
                            vector_sample_sizes, // number of elements in each vector of the input
                            OutputDim0, // number of output components
                            vector_sample_sizes + InputDim0); // number of elements in each vector of the output

    // at this point all NativeDataBuffers have been allocated, pointed to the correct locations
    // and buffers loaded with data from dataset_filename
}

void DataLoader::computeResult(std::vector<hebench::APIBridge::NativeDataBuffer *> &result,
                               const std::uint64_t *param_data_pack_indices,
                               hebench::APIBridge::DataType data_type)
{
    // as protected method, parameters should be valid when called

    assert(param_data_pack_indices[Index_W] == 0 && param_data_pack_indices[Index_b] == 0);

    // generate the output
    DataGeneratorHelper::logisticRegressionInference(data_type, m_polynomial_degree,
                                                     result.front()->p, // result
                                                     this->getParameterData(Index_W).p_buffers[0].p, // W
                                                     this->getParameterData(Index_b).p_buffers[0].p, // b
                                                     this->getParameterData(Index_X).p_buffers[param_data_pack_indices[Index_X]].p, // X
                                                     m_vector_size);
}
} // namespace LogisticRegression
} // namespace TestHarness
} // namespace hebench
