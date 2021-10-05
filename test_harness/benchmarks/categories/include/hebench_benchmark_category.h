
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_Benchmark_Category_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_Benchmark_Category_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "modules/general/include/nocopy.h"
#include "modules/logging/include/logging.h"

#include "include/hebench_ibenchmark.h"

namespace hebench {
namespace TestHarness {

class Engine;

class PartialBenchmarkCategory : public PartialBenchmark
{
public:
    DISABLE_COPY(PartialBenchmarkCategory)
    DISABLE_MOVE(PartialBenchmarkCategory)
private:
    IL_DECLARE_CLASS_NAME(PartialBenchmarkCategory)

public:
    typedef std::shared_ptr<PartialBenchmarkCategory> Ptr;

    ~PartialBenchmarkCategory() override;

protected:
    class RAIIHandle
    {
    public:
        DISABLE_COPY(RAIIHandle)

    public:
        RAIIHandle();
        RAIIHandle(RAIIHandle &&rhs) noexcept;
        RAIIHandle(const hebench::APIBridge::Handle &h) :
            handle(h) {}
        ~RAIIHandle();
        RAIIHandle &operator=(RAIIHandle &&rhs) noexcept;
        RAIIHandle &operator=(const hebench::APIBridge::Handle &rhs);
        operator bool() const noexcept { return isEmpty(this->handle); }
        void detach();
        hebench::APIBridge::ErrorCode destroy();

        hebench::APIBridge::Handle handle;

        static bool isEmpty(const hebench::APIBridge::Handle &h) noexcept;
    };

    PartialBenchmarkCategory(std::shared_ptr<Engine> p_engine,
                             const IBenchmarkDescription::DescriptionToken &description_token);

    /**
     * @brief Dataset to be used for operations previously initialized during
     * creation of this object.
     * @return Dataset to be used for operations.
     */
    virtual IDataLoader::Ptr getDataset() const = 0;
    /**
     * @brief Validates the result of an operation against the ground truth.
     * @param[in] dataset Dataset containing the data for the operation performed.
     * @param[in] param_data_pack_indices Collection of indices for data sample to
     * use inside each parameter pack. See IDataLoader::getResultIndex().
     * @param[in] p_outputs Pointer to the buffers containing the result of the
     * operation to validate.
     * @param[in] data_type Data type of the data used in the operation.
     * @returns `true` if result is valid.
     * @returns `false` if result is not valid (or an std::exception descendant can
     * be thrown in such case).
     * @details The number of buffers in vector \p p_outputs must be the same
     * as the dimension of the ground truth result. It is expected that each buffer
     * pointed by \p p_outputs is, at least, of the same size, in bytes, as the
     * corresponding ground truth buffer.
     *
     * Validation routine can either retrieve the ground truth result from \p dataset
     * or compute it on the fly using the inputs from \p dataset pointed by
     * \p param_data_pack_indices.
     *
     * The default implementation provided assumes that \p dataset contains the
     * results pre-computed and the data type for all arguments and results is
     * \p data_type. If this is not the case for the derived class, clients should
     * provide their own override version of this method.
     */
    virtual bool validateResult(IDataLoader::Ptr dataset,
                                const std::uint64_t *param_data_pack_indices,
                                const std::vector<hebench::APIBridge::NativeDataBuffer *> &outputs,
                                hebench::APIBridge::DataType data_type) const;
    /**
     * @brief Outputs the arguments, expected ground truth, and received result
     * to an output stream.
     * @param os Stream where to write the log.
     * @param dataset Dataset used for the operation.
     * @param[in] param_data_pack_indices Collection of indices for data sample to
     * use inside each parameter pack. See IDataLoader::getResultIndex().
     * @param[in] p_outputs Pointer to the buffers containing the result of the
     * operation to validate.
     * @param[in] data_type Data type of the data used in the operation.
     * @details The number of buffers in vector \p p_outputs must be the same
     * as the dimension of the ground truth result. It is expected that each buffer
     * pointed by \p p_outputs is, at least, of the same size, in bytes, as the
     * corresponding ground truth buffer.
     *
     * While not required, data output format should be a vector per column,
     * for uniformity with default implementation. Utility function
     * `hebench::Utilities::printArraysAsColumns()` can help with column format printout.
     *
     * The default implementation provided assumes that \p dataset contains the
     * results pre-computed and the data type for all arguments and results is
     * \p data_type. If this is not the case for the derived class, clients should
     * provide their own override version of this method.
     * @sa hebench::Utilities::printArraysAsColumns()
     */
    virtual void logResult(std::ostream &os,
                           IDataLoader::Ptr dataset,
                           const std::uint64_t *param_data_pack_indices,
                           const std::vector<hebench::APIBridge::NativeDataBuffer *> &outputs,
                           hebench::APIBridge::DataType data_type) const;
};

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_Benchmark_Category_H_0596d40a3cce4b108a81595c50eb286d
