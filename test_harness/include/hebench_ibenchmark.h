
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_IBenchmark_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_IBenchmark_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "hebench/modules/general/include/nocopy.h"
#include "hebench/modules/logging/include/logging.h"

#include "hebench/api_bridge/types.h"
#include "hebench_benchmark_description.h"
#include "hebench_idata_loader.h"
#include "hebench_utilities_harness.h"

namespace hebench {
namespace TestHarness {

class Engine;
class IBenchmark;
class PartialBenchmark;

/**
 * @brief Base interface for Benchmark Descriptors.
 * @details Benchmark Descriptors are lightweight classes that represent a benchmark
 * or collection of benchmarks that can be executed. For each supported benchmark,
 * a corresponding Benchmark Descriptor is registered with the BenchmarkFactory.
 *
 * During benchmark selection, BenchmarkFactory polls every registered Benchmark
 * Descriptor whether their represented benchmark can execute the benchmark described
 * by a backend. Once a descriptor is matched, it is responsible for creating its
 * represented benchmark object. Once the benchmark completes, the same descriptor is
 * tasked with destroying it.
 *
 * Clients should extend PartialBenchmarkDescriptor class when creating their descriptors
 * instead of this interface.
 */
class IBenchmarkDescriptor
{
public:
    DISABLE_COPY(IBenchmarkDescriptor)
    DISABLE_MOVE(IBenchmarkDescriptor)

public:
    /**
     * @brief Token returned by a successful call to IBenchmarkDescriptor::matchBenchmarkDescriptor().
     * @details Use the token returned by IBenchmarkDescriptor::matchBenchmarkDescriptor() to instantiate
     * the actual benchmark.
     *
     * When returned by IBenchmarkDescriptor::matchBenchmarkDescriptor(), this token fields
     * are fully and correctly set, including the backend `BenchmarkDescriptor` with concrete values
     * instead of configurable placeholder values. This token can be used to describe the benchmark.
     */
    class DescriptionToken
    {
    public:
        class FriendKeyCreation
        {
        public:
            friend class IBenchmarkDescriptor;

        private:
            FriendKeyCreation() {}
        };
        typedef std::shared_ptr<DescriptionToken> Ptr;

    public:
        DescriptionToken(IBenchmarkDescriptor &caller,
                         const BenchmarkDescription::Backend &backend_desc,
                         const BenchmarkDescription::Configuration &config,
                         const BenchmarkDescription::Description &text_desc,
                         const FriendKeyCreation &) :
            m_p_caller(&caller),
            m_backend_desc(backend_desc), m_config(config), m_description(text_desc)
        {
        }

        IBenchmarkDescriptor *getDescriptor() { return m_p_caller; }
        const BenchmarkDescription::Backend &getBackendDescription() const { return m_backend_desc; }
        const BenchmarkDescription::Configuration &getBenchmarkConfiguration() const { return m_config; }
        const BenchmarkDescription::Description &getDescription() const { return m_description; }

    private:
        IBenchmarkDescriptor *m_p_caller;
        BenchmarkDescription::Backend m_backend_desc;
        BenchmarkDescription::Configuration m_config;
        BenchmarkDescription::Description m_description;
    };

public:
    IBenchmarkDescriptor()          = default;
    virtual ~IBenchmarkDescriptor() = default;

    /**
     * @brief Determines if the represented benchmark can perform the workload described by
     * a specified HEBench benchmark descriptor and configuration.
     * @param[in] engine Engine requesting the matching.
     * @param[in] backend_desc Backend descriptor to match.
     * @param[in] config Configuration of benchmark to match.
     * @returns A valid token representing the matched benchmark if the benchmark corresponding
     * to this `IBenchmarkDescription` object is compatible with and can perform the described
     * workload.
     * @returns `null` otherwise.
     * @details
     * The token returned by this method can be passed to createBenchmark() to instantiate
     * the actual benchmark. All fields in the returned token must be fully and correctly set,
     * including the backend `BenchmarkDescriptor` with concrete values
     * instead of configurable placeholder values. This token can be used to describe the benchmark.
     *
     * This method is used by `BenchmarkFactory::createBenchmark()` to select
     * the appropriate benchmark to create based on the descriptor and the
     * workload parameters.
     */
    virtual DescriptionToken::Ptr matchDescriptor(const Engine &engine,
                                                  const BenchmarkDescription::Backend &backend_desc,
                                                  const BenchmarkDescription::Configuration &config) const = 0;

    /**
     * @brief Creates the represented IBenchmark object that can perform the workload
     * specified by the HEBench benchmark descriptor.
     * @param[in] p_engine The engine creating the benchmark.
     * @param[in] description_token Description token describing the benchmark to create,
     * as matched by matchBenchmarkDescriptor().
     * @return Pointer to object of class derived from `IBenchmark` that can perform the
     * specified workload or `null` on unknown error.
     * @details
     * Instead of returning `null`, this method may also throw instances of `std::exception`
     * to communicate errors to caller with better error descriptions.
     *
     * This method constructs an object of type derived from IBenchmark, performs any
     * pending initialization on the fully constructed object, and returns a pointer to it.
     * The returned pointer must live until passed to method `destroyBenchmark()`.
     * @sa destroyBenchmark()
     */
    virtual PartialBenchmark *createBenchmark(std::shared_ptr<Engine> p_engine,
                                              const DescriptionToken &description_token) = 0;
    /**
     * @brief Destroys an object returned by `createBenchmark()`.
     * @param[in] p_bench Pointer to object to clean up.
     * @sa createBenchmark()
     */
    virtual void destroyBenchmark(PartialBenchmark *p_bench) = 0;

protected:
    /**
     * @brief Creates a DescriptionToken object associated to this IBenchmarkDescription object
     * using the specified benchmark description.
     */
    DescriptionToken::Ptr createToken(const BenchmarkDescription::Backend &backend_desc,
                                      const BenchmarkDescription::Configuration &config,
                                      const BenchmarkDescription::Description &text_desc) const;

private:
    DescriptionToken::FriendKeyCreation m_key_creation;
};

/**
 * @brief Provides boilerplate implementation to common methods of interface
 * IBenchmarkDescription and implements some mechanisms to ease implementation
 * of the interface.
 * @details Clients looking to implement interface IBenchmarkDescription can derive
 * from this class.
 *
 * This class provides a default implementation to
 * `IBenchmarkDescription::matchBenchmarkDescriptor()`. This implementation calls
 * `PartialBenchmarkDescriptor::matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &)`,
 * and then, only if `true` is returned, it fills out the `Description` structure with
 * general information about the matched benchmark; `completeDescription()` is called
 * afterwards for clients to add or modify the general description.
 *
 * Clients need to implement:
 * - `IBenchmarkDescription::createBenchmark()`
 * - `IBenchmarkDescription::destroyBenchmark()`
 * - `PartialBenchmarkDescriptor::matchBenchmarkDescriptor()`
 * - `PartialBenchmarkDescriptor::completeWorkloadDescription()`
 */
class PartialBenchmarkDescriptor : public IBenchmarkDescriptor
{
public:
    DISABLE_COPY(PartialBenchmarkDescriptor)
    DISABLE_MOVE(PartialBenchmarkDescriptor)
private:
    IL_DECLARE_CLASS_NAME(PartialBenchmarkDescriptor)

public:
    /**
     * @brief Specifies whether frontend will override backend descriptors using
     * configuration data or not.
     * @returns `true` to have the frontend override the backend descriptors.
     * @returns `false` to have the frontend respect the backend descriptors.
     * @details When `true`, frontend implementations should attempt to override
     * backend descriptors with the corresponding configuration values. In this case,
     * it is expected that backend implementations will validate the concrete
     * benchmark descriptor during `hebench::APIBridge::initBenchmark()` to detect
     * invalid configurations.
     *
     * If `false`, frontend implementations should maintain non-flexible values of
     * backend descriptors and ignore respective configuration data.
     */
    static bool getForceConfigValues() { return m_b_force_config_value; }
    /**
     * @brief Sets whether frontend will override backend descriptors using
     * configuration data or not.
     * @param[in] value New value to set.
     * @details When set to `true`, frontend implementations should attempt to override
     * backend descriptors with the corresponding configuration values. In this case,
     * it is expected that backend implementations will validate the concrete
     * benchmark descriptor during `hebench::APIBridge::initBenchmark()` to detect
     * invalid configurations.
     *
     * If set to `false`, frontend implementations should maintain non-flexible values of
     * backend descriptors and ignore respective configuration data.
     */
    static void setForceConfigValues(bool value) { m_b_force_config_value = value; }

    /**
     * @brief Extracts the batch sizes for a workload from a specified HEBench API
     * benchmark descriptor.
     * @param[out] sample_sizes Array with \p param_count elements to receive the sample
     * sizes for each operation parameter. This is the final value to be used during testing.
     * @param[in] param_count Number of parameters for the workload operation.
     * @param[in] config_sample_sizes User-requested (configured) sample sizes per operand.
     * @param[in] bench_desc HEBench API benchmark descriptor from which to extract the
     * workload batch sizes.
     * @param[in] default_sample_size_fallback Fallback sample size for when none is
     * specified in \p bench_desc or \p default_sample_sizes . Must be greater than `0`.
     * @param[in] force_config If `true` \p default_sample_sizes values have priority
     * over the those in \p bench_desc `.cat_params.offline.data_count`
     * @return The batch size for the result of the operation.
     * @details For an operation parameter the sample size priority is:
     *
     * For `force_config == false`
     *
     * 1. backend specified (`bench_desc.cat_params.offline.data_count[i]`)
     * 2. benchmark specific in config file (`config_sample_sizes[i]`)
     * 3. global config file
     * 4. workload specification
     *
     * For `force_config == false`
     *
     * 1. benchmark specific in config file (`config_sample_sizes[i]`)
     * 2. backend specified (`bench_desc.cat_params.offline.data_count[i]`)
     * 3. global config file
     * 4. workload specification
     *
     * If any of these is `0`, then the next down the list is used. Workload specification
     * must always be greater than `0`. Workload specification sizes are always supported.
     * Sample sizes from any other sources are requests and may be satisfied based on
     * actual dataset size.
     */
    static std::uint64_t computeSampleSizes(std::uint64_t *sample_sizes,
                                            std::size_t param_count,
                                            const std::vector<std::uint64_t> &config_sample_sizes,
                                            const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                            std::uint64_t default_sample_size_fallback,
                                            bool force_config);

public:
    PartialBenchmarkDescriptor();
    ~PartialBenchmarkDescriptor() override;

    /**
     * @brief Implementation of IBenchmarkDescriptor::matchDescriptor().
     * @details
     * Clients cannot override this method.
     *
     * This method calls the internal PartialBenchmarkDescriptor::matchBenchmarkDescriptor()
     * which clients must implement. If matchBenchmarkDescriptor() returns `true`, then method
     * PartialBenchmarkDescriptor::completeWorkloadDescription() is called to complete
     * the description with information specific to the workload.
     * @sa PartialBenchmarkDescriptor::matchBenchmarkDescriptor(), IBenchmarkDescriptor::matchDescriptor()
     */
    DescriptionToken::Ptr matchDescriptor(const Engine &engine,
                                          const BenchmarkDescription::Backend &backend_desc,
                                          const BenchmarkDescription::Configuration &config) const override final;

protected:
    static std::unordered_set<std::size_t> getCipherParamPositions(std::uint32_t cipher_param_mask);
    static std::string getCategoryName(hebench::APIBridge::Category category);
    static std::string getDataTypeName(hebench::APIBridge::DataType data_type);

protected:
    /**
     * @brief Bundles values that need to be filled by a workload during
     * `completeWorkloadDescription()`.
     * @sa completeWorkloadDescription()
     */
    struct WorkloadDescriptionOutput
    {
        /**
         * @brief Number of parameters for the represented workload operation.
         */
        std::size_t operation_params_count;
        /**
         * @brief Benchmark descriptor completed with concrete values assigned to configurable fields.
         */
        hebench::APIBridge::BenchmarkDescriptor concrete_descriptor;
        /**
         * @brief Human-readable friendly name for the represented workload to be used for
         * its description on the report.
         */
        std::string workload_base_name;
        /**
         * @brief Human-readable friendly name for the represented workload to be used for
         * its description on the report.
         */
        std::string workload_name;
        /**
         * @brief Workload specific information to be added to the report header.
         * @details If this is not the empty string, it will be appended in the report to
         * a pre-generated header in CSV format.
         */
        std::string workload_header;
    };

    /**
     * @brief Determines if the represented benchmark can perform the workload described by
     * a specified HEBench benchmark descriptor and workload parameters.
     * @param[in] bench_desc Descriptor to which compare.
     * @param[in] w_params Arguments for the workload parameters.
     * @returns true if the represented benchmark can perform the specified described workload.
     * @returns false if no match was found.
     * @details
     * This method is used by `BenchmarkFactory::createBenchmark()` to select
     * the appropriate benchmark to create based on the descriptor.
     * @sa `IBenchmarkDescriptor::matchDescriptor()`
     */
    virtual bool matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                          const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const = 0;

    /**
     * @brief Completes the description for the matched benchmark.
     * @param[out] output Structure to receive the completed description.
     * @param[in] engine Engine that requested the matching.
     * @param[in] backend_desc Backend descriptor to be described.
     * @param[in] config Configuration for the benchmark being described.
     * @throws std::invalid_argument if fields in \p backend_desc or \p config are not
     * supported or invalid.
     * @details This method will only be called if
     * `PartialBenchmarkDescriptor::matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &, const std::vector<hebench::APIBridge::WorkloadParam> &)`
     * returns `true`.
     *
     * All fields in \p config are valid. In \p backend_desc: field `operation_params_count`
     * is expected to be returned by this method; field `descriptor` is the original returned
     * from the backend, and it is expected that this method returns the final, concrete version.
     * All other input parameters are valid. Use these values to complete all fields in \p output.
     *
     * This method must fill out all fields in the \p output structure. Field `workload_header`
     * may be set to empty string, but `workload_name` and `operation_params_count` must be
     * set to the correct values. Field `concrete_descriptor` must be set to the completed
     * benchmark descriptor, where all configurable values are replaced by the concrete, final values.
     * See `WorkloadDescriptionOutput` for more details.
     *
     * If extra header information returned in `workload_header` is not empty, it will be
     * appended in the report to a pre-generated header in CSV format. The following is an
     * example of a pre-generated header:
     *
     * @code
     * Specifications,
     * , Encryption,
     * , , Scheme, CKKS
     * , , Security, 128 bits
     * , , Poly mod degree, 8192
     * , , Primes, 3
     *
     * , Category, Latency
     * , , Warmup iterations, 2
     * , , Minimum test time (ms), 2000
     *
     * , Workload, Element-wise Vector Addition (1000)
     * , , Data type, Float64
     * , , Encrypted parameters, All
     * @endcode
     *
     * \p output `.workload_header` will be appended at the next line immediately after this.
     * @sa `WorkloadDescriptionOutput`
     */
    virtual void completeWorkloadDescription(WorkloadDescriptionOutput &output,
                                             const Engine &engine,
                                             const BenchmarkDescription::Backend &backend_desc,
                                             const BenchmarkDescription::Configuration &config) const = 0;

    /**
     * @brief Completes common elements of category parameters in a descriptor
     * using the specified configuration.
     * @param out_descriptor
     * @param in_descriptor
     * @param config
     * @param force_config
     * @details It sets the common values of cat_params based on the force config
     * policy.
     *
     * Common `cat_params` fields:
     *
     * - `cat_params.min_test_time_ms`
     */
    static void completeCategoryParams(hebench::APIBridge::BenchmarkDescriptor &out_descriptor,
                                       const hebench::APIBridge::BenchmarkDescriptor &in_descriptor,
                                       const BenchmarkDescription::Configuration &config,
                                       bool force_config);

private:
    static bool m_b_force_config_value;

    /**
     * @brief Completes the description of the benchmark.
     * @param[out] concrete_backend_desc Concrete, finalized backend description
     * as completed by the frontend benchmark from the original backend descriptor and
     * configuration parameters.
     * @param[out] concrete_config Concrete, finalized configuration for the benchmark
     * as completed by the frontend benchmark from the original backend descriptor and
     * configuration parameters.
     * @param[out] text_desc Friendly human-readable description of the benchmark.
     * @param[in] engine Engine.
     * @param[in] backend_desc Original benchmark description as returned by backend.
     * @param[in] config Original configuration parameters for this benchmark as read from
     * configuration file (or default parameters if no configuration file was supplied).
     */
    void describe(BenchmarkDescription::Backend &concrete_backend_desc,
                  BenchmarkDescription::Configuration &concrete_config,
                  BenchmarkDescription::Description &text_desc,
                  const Engine &engine,
                  const BenchmarkDescription::Backend &backend_desc,
                  const BenchmarkDescription::Configuration &config) const;
};

/**
 * @brief Interface for benchmarks.
 * @details To implement this interface and create workloads, it is easier to extend
 * directly from the workload category benchmark classes BenchmarkLatency, BenchmarkOffline,
 * etc. If more control or specific processing other than the generic category execution
 * is needed, extending from PartialBenchmark offers more flexibility.
 */
class IBenchmark
{
public:
    DISABLE_COPY(IBenchmark)
    DISABLE_MOVE(IBenchmark)
private:
    IL_DECLARE_CLASS_NAME(IBenchmark)

public:
    typedef std::shared_ptr<IBenchmark> Ptr;
    /**
     * @brief Provides configuration to and retrieves data from a benchmark run.
     */
    struct RunConfig
    {
        /**
        * @brief Specifies whether the benchmark will validate backend results (`true`) or
        * it will simply benchmark without validating (`false`).
        * @details Shutting down validation can be useful when re-running benchmarks
        * that have been already validated or when creating and debugging new backends.
        */
        bool b_validate_results;
    };

    virtual ~IBenchmark() = default;

    /**
     * @brief Executes the benchmark operations.
     * @param out_report Object where to append the report of the operation.
     * @param config Specifies configuration parameters for the run.
     * @returns true if benchmark succeeded and operation results were valid.
     * @returns false if benchmark operation results were not valid.
     * @details
     * <b>Convention:</b>
     *
     * - Returns when no errors (`true`), or validation failed (`false`).
     * - hebench::Common::ErrorException with backend error number attached if
     * backend errors occur.
     * - Throws std::exception (or derived exception other than hebench::Common::ErrorException)
     * on any other errors.
     */
    virtual bool run(hebench::Utilities::TimingReportEx &out_report, RunConfig &config) = 0;

    virtual std::weak_ptr<Engine> getEngine() const          = 0;
    virtual const hebench::APIBridge::Handle &handle() const = 0;

protected:
    IBenchmark() = default;
};

/**
 * @brief Base class for benchmarks.
 * @details This class offers common implementations to trivial methods declared
 * by interface IBenchmark.
 *
 * To extend this class and create workloads, it is easier to extend directly from the
 * workload category benchmark classes BenchmarkLatency, BenchmarkOffline, etc.
 * If more control or specific processing other than the generic category execution
 * is needed, extending from this class offers more flexibility.
 */
class PartialBenchmark : public IBenchmark
{
public:
    DISABLE_COPY(PartialBenchmark)
    DISABLE_MOVE(PartialBenchmark)
private:
    IL_DECLARE_CLASS_NAME(PartialBenchmark)

public:
    class FriendPrivateKey
    {
        friend class BenchmarkFactory;

    private:
        FriendPrivateKey() {}
    };

public:
    typedef std::shared_ptr<PartialBenchmark> Ptr;

    ~PartialBenchmark() override;

    std::weak_ptr<Engine> getEngine() const override { return m_p_engine; }
    const hebench::APIBridge::Handle &handle() const override { return m_handle; }
    /**
     * @brief An ID to identify the first event during the benchmark run.
     * @return ID of first event of the benchmark run.
     * @details All events in the execution of the benchmark will have an ID.
     * The first event's ID will be this value. ID of subsequent events will
     * be a value greater than this ID.
     *
     * This is for report and display purposes only, so it matters not whether
     * different benchmarks produce the same ID. If no override, it defaults
     * to 0 for all derived classes.
     */
    virtual std::uint32_t getEventIDStart() const { return 0; }
    /**
     * @brief Returns the next available event ID.
     * @return An integer representing the next available event ID that has
     * not been returned before.
     * @details Next available event ID starts from `getEventIDStart()` and
     * increments by one on every call to this method. This ensures unique
     * event IDs for an object of this class.
     */
    std::uint32_t getEventIDNext()
    {
        if (m_b_constructed && m_current_event_id < getEventIDStart())
            m_current_event_id = getEventIDStart();
        return ++m_current_event_id;
    }

    /**
     * @brief Initializes the partial benchmark members.
     * @details This method is provided to allow clients to perform polymorphic
     * initialization outside of the class constructor before the backend benchmark
     * is initialized. Unless an initialized backend benchmark is required, this is
     * a good spot to perform dataset generation or loading for the benchmark to execute.
     *
     * This method is called automatically during creation and initialization
     * of the benchmark.
     */
    virtual void init() = 0;
    /**
     * @brief Initializes backend benchmark.
     * @details HEBench API calls are performed here in order to initialize the
     * benchmark corresponding to the description token used during construction.
     */
    void initBackend(hebench::Utilities::TimingReportEx &out_report, const FriendPrivateKey &);

    /**
     * @brief Called automatically during initialization after the backend has
     * been initialized.
     * @details This method allows clients to perform polymorphic
     * initialization outside of the class constructor after the backend benchmark
     * has been initialized, if needed.
     *
     * Derived classes MUST call this base implementation as first step when overriding,
     * otherwise, `checkInitializationState()` will throw exceptions.
     *
     * This method is called automatically during creation and initialization
     * of the benchmark.
     * @sa checkInitializationState()
     */
    virtual void postInit();

    /**
     * @brief Used to check that initialization steps have been completed successfully.
     * @throws std::runtime_error if initialization of `PartialBenchmark` is incomplete.
     * @details During benchmark creation and initialization the following methods are
     * called in order automatically by the framework:
     * @code
     * (constructor)
     * init()
     * initBackend()
     * postInit()
     * checkInitializationState()
     * @endcode
     *
     * If any existing default base implementation of these methods provided by
     * PartialBenchmark has not been called, then this method will fail and the exception
     * will be thrown.
     * @sa init(), initBackend(), postInit()
     */
    void checkInitializationState(const FriendPrivateKey &) const;

protected:
    /**
     * @brief Allows read-only access to this benchmark backend description.
     */
    const BenchmarkDescription::Backend &getBackendDescription() const { return m_backend_desc; }
    /**
     * @brief Allows read-only access to this benchmark configuration.
     */
    const BenchmarkDescription::Configuration &getBenchmarkConfiguration() const { return m_config; }
    /**
     * @brief Allows read-only access to this benchmark text description.
     */
    const BenchmarkDescription::Description &getDescription() const { return m_text_description; }

    PartialBenchmark(std::shared_ptr<Engine> p_engine,
                     const IBenchmarkDescriptor::DescriptionToken &description_token);

    /**
     * @brief Validates whether the specified HEBench API return code represents a
     * success or error.
     * @param[in] err_code Error code to validate.
     * @param[in] last_error Specifies if this is the latest error to occur.
     * @throws std::runtime_error if \p err_code represents an error.
     * @details If \p err_code represents success, this method returns immediately.
     * Otherwise, a std::runtime_error exception is thrown with the error description
     * and extra information as retrieved from the backend if requested by setting
     * \p last_error to `true`.
     */
    void validateRetCode(hebench::APIBridge::ErrorCode err_code, bool last_error = true) const;

private:
    void internalInit(const IBenchmarkDescriptor::DescriptionToken &description_token);

    std::shared_ptr<Engine> m_p_engine;
    hebench::APIBridge::Handle m_handle;
    BenchmarkDescription::Backend m_backend_desc;
    BenchmarkDescription::Configuration m_config;
    BenchmarkDescription::Description m_text_description;
    std::uint32_t m_current_event_id;
    bool m_b_constructed;
    bool m_b_initialized;
};

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_IBenchmark_H_0596d40a3cce4b108a81595c50eb286d
