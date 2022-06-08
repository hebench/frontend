
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_IDataLoader_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_IDataLoader_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>
#include <string>
#include <vector>

#include "modules/general/include/nocopy.h"
#include "modules/logging/include/logging.h"

#include "hebench/api_bridge/types.h"
#include "hebench_types_harness.h"

namespace hebench {
namespace TestHarness {

class IDataLoader
{
public:
    DISABLE_COPY(IDataLoader)
    DISABLE_MOVE(IDataLoader)
private:
    IL_DECLARE_CLASS_NAME(IDataLoader)

public:
    template <typename T>
    using unique_ptr_custom_deleter = hebench::TestHarness::unique_ptr_custom_deleter<T>;

    struct ResultData
    {
        /**
         * @brief Points to the `NativeDataBuffer` s containing the result sample for
         * an input sample.
         */
        std::vector<const hebench::APIBridge::NativeDataBuffer *> result;
        /**
         * @brief Index of the result sample.
         */
        std::uint64_t sample_index;
        std::shared_ptr<void> reserved0; // use for RAII if needed
    };
    typedef std::shared_ptr<ResultData> ResultDataPtr;

    typedef std::shared_ptr<IDataLoader> Ptr;

    virtual ~IDataLoader() {}

    static std::size_t sizeOf(hebench::APIBridge::DataType data_type);

    /**
     * @brief Creates shallow packed data that self cleans up.
     * @param[in] data_pack_count Number of DataPack objects that will be pointed to
     * by this DataPackCollection.
     * @return A smart pointer to a DataPackCollection structure.
     * @details The returned DataPackCollection will self clean up its p_data_packs field
     * when it goes out of scope.
     *
     * Clients can use the created DataPackCollection by pointing to their DataPack objects
     * in the pre-allocated p_data_packs field array.
     */
    static unique_ptr_custom_deleter<hebench::APIBridge::DataPackCollection>
    createDataPackCollection(std::uint64_t data_pack_count);
    /**
     * @brief Creates shallow data pack that self cleans up.
     * @param[in] buffer_count Number of NativeDataBuffer objects pointed to by this
     * DataPack.
     * @param[in] param_position Parameter position of this DataPack.
     * @return A smart pointer to a DataPack structure.
     * @details The returned DataPack will self clean up its p_buffers field
     * when it goes out of scope.
     *
     * Clients can use the created DataPack by pointing to their NativeDataBuffer objects
     * in the pre-allocated p_buffers field array.
     */
    static unique_ptr_custom_deleter<hebench::APIBridge::DataPack>
    createDataPack(std::uint64_t buffer_count, std::uint64_t param_position);
    static unique_ptr_custom_deleter<hebench::APIBridge::NativeDataBuffer>
    createDataBuffer(std::uint64_t size, std::int64_t tag);

    /**
     * @brief Number of parameter components (operands) for the represented operation.
     * @details This is the number of operands for the operation. For example, a unary
     * operation has 1 operand, binary has 2, n-ary has n.
     */
    virtual std::uint64_t getParameterCount() const = 0;
    /**
     * @brief Data pack for specified operation parameter (operand).
     * @param[in] param_position Zero-based position of the parameter.
     * @return A data pack containing the collection of samples for the specified parameter.
     * @details The parameter position for an operation is zero-based starting from the
     * leftmost parameter. i.e. For an operation such as:
     *
     * @code
     * R = op(A, B, C, ...)
     * @endcode
     *
     * R is the result, A is parameter 0, B, is parameter 1, C is parameter 2, etc.
     */
    virtual const hebench::APIBridge::DataPack &getParameterData(std::uint64_t param_position) const = 0;
    /**
     * @brief Number of components in a result for the represented operation.
     * @details Shape of result is always 2D: [getResultCount(), ?].
     *
     * For example, if the represented operation has getParameterCount() = 3 input
     * parameters and getResultCount() = 2 outputs, then, the shape of the result is
     * @code
     * [2, getParameterData(0).buffer_count * getParameterData(1).buffer_count * getParameterData(2).buffer_count]
     * @endcode
     */
    virtual std::uint64_t getResultCount() const = 0;
    /**
     * @brief Data pack corresponding to the specified component of the result.
     * @param[in] param_position Zero-based position of the component to return.
     * @return The data pack containing the collection of samples for the specified
     * component of the result.
     */
    virtual const hebench::APIBridge::DataPack &getResultData(std::uint64_t param_position) const = 0;

    /**
     * @brief getResultFor
     * @param[in] param_data_pack_indices Collection of indices for data sample to
     * use inside each parameter pack. Number of elements pointed must be, at least,
     * `getParameterCount()`.
     * @return Returns a non-null pointer ResultData containing the ground-truth result
     * corresponding to the specified parameter indices.
     * @throws std::out_of_range if any index is out of range.
     * @throws std::invalid_argument if \p param_data_pack_indices is null.
     * @throws instance of std::exception on any other error.
     * @details The shape of result is always 2D: [n = getResultCount(), ?], so, the result
     * for an operation is
     * @code
     * (result[0][r_i], result[1][r_i], ..., result[n-1][r_i])
     * @endcode
     * where r_i is the index of the `NativeDataBuffer`s for the result in the second dimension.
     * @sa getResultIndex()
     */
    virtual ResultDataPtr getResultFor(const std::uint64_t *param_data_pack_indices) = 0;
    /**
     * @brief Computes the index of the result NativeDataBuffer given the indices
     * of the input data.
     * @param[in] param_data_pack_indices Collection of indices for data sample to
     * use inside each parameter pack. Number of elements pointed must be, at least,
     * `parameterCount()`.
     * @return Returns the index for the ground-truth result corresponding to the
     * specified parameter indices.
     * @throw std::out_of_range if any index is out of range.
     * @throws std::invalid_argument if \p param_data_pack_indices is null.
     * @details The shape of result is always 2D: [n = resultCount(), ?], so, the result
     * for an operation is
     * @code
     * (result[0][r_i], result[1][r_i], ..., result[n-1][r_i])
     * @endcode
     * where r_i is the index of the data buffer for the result in the second dimension.
     *
     * By specification definition, the result index (r_i) is computed in a row-major fashion,
     * where the most significant parameter moves faster.
     *
     * In general:
     *
     * @code
     * r_i = param_data_pack_indices[0]
     * for i in [1..getParameterCount() - 1]
     *      r_i = param_data_pack_indices[i] + getParameterData(i).buffer_count * r_i;
     * @endcode
     *
     * For example, given the operation `op()` that returns a result of the shape [1, ?]:
     *
     * @code
     * param_count[0] = 3;
     * param_count[1] = 10;
     * Param[0][param_count[0]]
     * Param[1][param_count[1]]
     *
     * param_data_pack_indices[0] = 2
     * param_data_pack_indices[1] = 3
     * Result = op(param_data_pack_indices[0], param_data_pack_indices[1]);
     * @endcode
     *
     * Then, `r_i`, the index in the second dimension that corresponds to the NativeDataBuffer
     * in `getResultData(0)` where the result of the operation using the specified indices
     * will be placed, is computed in row-major order as:
     *
     * @code
     * r_i = param_data_pack_indices[0] * param_count[1] + param_data_pack_indices[1]
     * r_i = 2 * 10 + 3 = 23
     * @endcode
     *
     * then, `Result` is actually:
     *
     * @code
     * Result = [[ getResultData(0).p_buffers[r_i] ]]
     *
     * or
     *
     * Result = [[ getResultData(0).p_buffers[23]  ]]
     * @endcode
     *
     * For complete details, see \ref results_order .
     * @sa hebench::APIBridge::Category::Offline
     */
    virtual std::uint64_t getResultIndex(const std::uint64_t *param_data_pack_indices) const = 0;

    /**
     * @brief Total data loaded by this loader in bytes.
     */
    virtual std::uint64_t getTotalDataLoaded() const = 0;

protected:
    IDataLoader() {}
};

/**
 * @brief Base class for data loaders and data generators.
 * @details During initialization, derived class should call
 * PartialDataLoader::init() to initialize the dimensions of the inputs and outputs.
 * After initializing the dimensions, the derived class should call
 * PartialDataLoader::allocate() to allocate the data for the inputs and outputs
 * specified during PartialDataLoader::init().
 *
 * After the data has been allocated, the methods PartialDataLoader::getParameterCount(),
 * PartialDataLoader::getParameterData(), PartialDataLoader::getResultCount(),
 * PartialDataLoader::getResultData() and other data access methods become available,
 * and the buffers will be pointed to the correct memory location to contain
 * the data specified. Derived class can, then use these methods to generate
 * the data or load from storage into the allocated memory.
 */
class PartialDataLoader : public IDataLoader
{
public:
    DISABLE_COPY(PartialDataLoader)
    DISABLE_MOVE(PartialDataLoader)
private:
    IL_DECLARE_CLASS_NAME(PartialDataLoader)

    template <typename>
    friend class PartialDataLoaderHelper;

public:
    typedef std::shared_ptr<PartialDataLoader> Ptr;

    ~PartialDataLoader() override {}

    std::uint64_t getParameterCount() const override { return m_input_data.size(); }
    const hebench::APIBridge::DataPack &getParameterData(std::uint64_t param_position) const override;
    std::uint64_t getResultCount() const override { return m_output_data.size(); }
    const hebench::APIBridge::DataPack &getResultData(std::uint64_t param_position) const override;
    ResultDataPtr getResultFor(const std::uint64_t *param_data_pack_indices) override;
    std::uint64_t getResultIndex(const std::uint64_t *param_data_pack_indices) const override;
    std::uint64_t getTotalDataLoaded() const override { return m_raw_buffer.size(); }
    /**
     * @brief Retrieves whether buffers to contain output data have been allocated or not.
     * @returns true is memory is allocated for outputs.
     * @returns false otherwise.
     * @details
     * If false, no memory was allocated to hold ground truth results during initialization.
     *
     * When calling methods `getResultData()` or `getResultFor()`, the returned buffers in
     * `NativeDataBuffer::p` will be null. However, all other fields will be valid in order
     * to provide information regarding output dimensions, size required to hold output data,
     * etc. Clients may use `getResultTempDataPacks()` method to obtain a temporary scratch
     * memory allocation to hold a single result sample.
     * @sa init(), getResultTempDataPacks()
     */
    bool hasResults() const { return m_b_is_output_allocated; }
    bool isInitialized() const { return m_b_initialized; }
    hebench::APIBridge::DataType getDataType() const { return m_data_type; }

protected:
    PartialDataLoader();
    /**
     * @brief Initializes dimensions of inputs and outputs. No allocation is performed.
     * @param[in] data_type Data type of elements contained in the samples for this dataset.
     * @param[in] input_dim Dimension of the input (to become getParameterCount()).
     * @param[in] input_sample_count_per_dim For each dimension in the input, how many
     * NativeDataBuffer samples will be loaded, this is, sample size for the input dimension.
     * Must point to a buffer with enough space to contain, at least, \p input_dim values.
     * @param[in] input_count_per_dim Array with the number of elements of type
     * \p data_type for each sample in the corresponding input parameter.
     * Must point to a buffer with enough space to contain, at least, \p input_dim values.
     * @param[in] output_dim Dimension of the output (to become getResultCount()).
     * @param[in] output_count_per_dim Array with the number of elements of type
     * \p data_type for each sample in the corresponding output dimension.
     * Must point to a buffer with enough space to contain, at least, \p output_dim values.
     * @param[in] allocate_output Specifies whether buffers for output ground truth will be
     * allocated.
     * @throws instance of, or derived from std::exception on error.
     * @details After this call succeeds, all buffers for inputs and outputs have been
     * allocated and pointed to the correct locations. It is responsibility of caller to
     * initialize the values of the allocated buffers.
     *
     * If \p allocate_output is false, no memory is allocated to hold ground truth results.
     * When calling methods `getResultData()` or `getResultFor()`, the returned buffers in
     * `NativeDataBuffer::p` will be null. However, all other fields will be valid in order
     * to provide information regarding output dimensions, size required to hold output data,
     * etc. Clients may use `getResultTempDataPacks()` method to obtain a temporary scratch
     * memory allocation to hold a single result sample.
     */
    void init(hebench::APIBridge::DataType data_type,
              std::size_t input_dim,
              const std::size_t *input_sample_count_per_dim,
              const std::uint64_t *input_count_per_dim,
              std::size_t output_dim,
              const std::uint64_t *output_count_per_dim,
              bool allocate_output);
    /**
     * @brief Loads a dataset from a file.
     * @param[in] filename File containing the dataset input and, optionally, expected
     * outputs. The file must be one of the supported loader formats.
     * @param[in] data_type Type of the data for the benchmark that will use the dataset.
     * @param[in] expected_input_dim Dimension of the input (to become getParameterCount()).
     * @param[in] max_input_sample_count_per_dim Maximum sample size requested for each
     * input dimension.
     * Must point to a buffer with enough space to contain, at least, \p expected_input_dim values.
     * @param[in] expected_input_count_per_dim Expected number of elements of type \p data_type
     * in an input sample vector for the corresponding dimension.
     * Must point to a buffer with enough space to contain, at least, \p expected_input_dim values.
     * @param[in] expected_output_dim Dimension of the output (to become getResultCount()).
     * @param[in] expected_output_count_per_dim Expected number of elements of type
     * \p data_type for each sample in the corresponding output dimension.
     * Must point to a buffer with enough space to contain, at least, \p expected_output_dim values.
     * @throws instance of, or derived from std::exception on error.
     * @details Note that \p max_input_sample_count is an upper bound on the number of
     * input samples per input component. Depending on the dataset file loaded, the actual
     * sample size for a component may be less than the specified max value.
     *
     * The number of output samples is the multiplication of the number of samples per input
     * component.
     *
     * On error, this method throws an instance of, or derived from std::exception.
     */
    void init(const std::string &filename,
              hebench::APIBridge::DataType data_type,
              std::size_t expected_input_dim,
              const std::size_t *max_input_sample_count_per_dim,
              const std::uint64_t *expected_input_count_per_dim,
              std::size_t expected_output_dim,
              const std::uint64_t *expected_output_count_per_dim);

    /**
     * @brief Retrieves a pre-allocated result providing memory space to store
     * a single operation result sample.
     * @param[in] result_index Result sample index on which to base the pre-allocated
     * buffers.
     * @return Pre-allocated collection of `DataPack`. All fields in the Data packs and
     * contained `NativeDataBuffer` are valid. Do not assign new values to any fields.
     * @throws std::logic_error if this method is called before `init()` has been called.
     * @throws std::out_of_range if `result_index` out of range.
     * @details This method must be called after `init()` has been called.
     *
     * Clients can use the memory pointed to by the contained `NativeDataBuffer`
     * to store the values of the corresponding result sample. Technically, buffers allocated
     * for every possible `result_index` should have the same capacity, thus, requesting
     * allocation for result `0` should work for all results.
     *
     * Allocation occurs every time this method is called.
     *
     * Returned Data packs and associated buffers will be automatically cleaned up when
     * out of scope and no other copies are available.
     */
    std::vector<std::shared_ptr<hebench::APIBridge::DataPack>> getResultTempDataPacks(std::uint64_t result_index) const;
    /**
     * @brief Retrieves a pre-allocated result providing memory space to store
     * a single operation result sample.
     * @param[in] param_data_pack_indices For each operation parameter, this array
     * indicates the sample index to use for the operation.
     * Must contain, at least, `getParameterCount()` elements.
     * @return Pre-allocated collection of `DataPack`. All fields in the Data packs and
     * contained `NativeDataBuffer` are valid. Do not assign new values to any fields.
     * @throws std::logic_error if this method is called before `init()` has been called.
     * @throws std::out_of_range if values in `param_data_pack_indices` fall out of range.
     * @details This method must be called after `init()` has been called.
     *
     * Clients can use the memory pointed to by the contained `NativeDataBuffer`
     * to store the values of the corresponding result sample. Technically, buffers allocated
     * for every possible result sample should have the same capacity, thus, requesting
     * allocation for first result sample should work for all results.
     *
     * Allocation occurs every time this method is called.
     *
     * Returned Data packs and associated buffers will be automatically cleaned up when
     * out of scope and no other copies are available.
     */
    std::vector<std::shared_ptr<hebench::APIBridge::DataPack>> getResultTempDataPacks(const std::uint64_t *param_data_pack_indices) const
    {
        return getResultTempDataPacks(getResultIndex(param_data_pack_indices));
    }
    /**
     * @brief Retrieves a pre-allocated result providing memory space to store
     * a single operation result sample.
     * @return Pre-allocated collection of `DataPack`. All fields in the Data packs and
     * contained `NativeDataBuffer` are valid. Do not assign new values to any fields.
     * @throws std::logic_error if this method is called before `init()` has been called.
     * @details This method must be called after `init()` has been called.
     *
     * Clients can use the memory pointed to by the contained `NativeDataBuffer`
     * to store the values of the corresponding result sample. The returned allocation
     * is appropriate to store the first result sample of the operation based on the
     * first input sample. Technically, buffers allocated
     * for every possible result sample should have the same capacity, thus, requesting
     * allocation for first result sample should work for all results.
     *
     * Allocation occurs every time this method is called.
     *
     * Returned Data packs and associated buffers will be automatically cleaned up when
     * out of scope and no other copies are available.
     */
    std::vector<std::shared_ptr<hebench::APIBridge::DataPack>> getResultTempDataPacks() const
    {
        return getResultTempDataPacks(0UL);
    }

private:
    /**
     * @brief Allocates the actual memory for the inputs and outputs data buffers.
     * @param[in] input_buffer_sizes Array with the sizes, in bytes, for the data
     * buffer of each input parameter, in order.
     * @param[in] input_buffer_sizes_count Number of elements in array \p input_buffer_sizes.
     * Must be, at least, getParameterCount().
     * @param[in] output_buffer_sizes Array with the sizes, in bytes, for the data
     * buffer of each output, in order.
     * @param[in] output_buffer_sizes_count Number of elements in array \p output_buffer_sizes.
     * Must be, at least, getResultCount().
     * @param[in] allocate_output Specifies whether buffers for output ground truth will be
     * allocated.
     * @details
     * If \p allocate_output is false, this method will not allocate memory to hold ground
     * truth values. In this case, NativeDataBuffer::p inside result DataPacks will point to
     * null; however, NativeDataBuffer::size will be correctly initialized. Users must
     * calculate the ground truths based on the input samples on the fly or use their own
     * custom storage to pre-cache ground truth.
     */
    void allocate(const std::uint64_t *input_buffer_sizes,
                  std::size_t input_buffer_sizes_count,
                  const std::uint64_t *output_buffer_sizes,
                  std::size_t output_buffer_sizes_count,
                  bool allocate_output);

    std::vector<unique_ptr_custom_deleter<hebench::APIBridge::DataPack>> m_input_data;
    // output data is ordered such that each element of the data pack
    // is the result of the input parameters picked in row major order
    std::vector<unique_ptr_custom_deleter<hebench::APIBridge::DataPack>> m_output_data;
    // RAII storage for native data buffers
    std::vector<std::uint8_t> m_raw_buffer; // used to store all the data in contiguous memory
    hebench::APIBridge::DataType m_data_type;
    bool m_b_is_output_allocated;
    bool m_b_initialized;
};

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_IDataLoader_H_0596d40a3cce4b108a81595c50eb286d
