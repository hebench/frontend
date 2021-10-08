
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Harness_IBenchmark_H_0596d40a3cce4b108a81595c50eb286d
#define _HEBench_Harness_IBenchmark_H_0596d40a3cce4b108a81595c50eb286d

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "modules/general/include/nocopy.h"
#include "modules/logging/include/logging.h"

#include "hebench/api_bridge/types.h"
#include "hebench_idata_loader.h"
#include "hebench_utilities.h"

namespace hebench {
namespace TestHarness {

class BenchmarkFactory;
class Engine;
class IBenchmark;
class PartialBenchmark;

class IBenchmarkDescription
{
public:
    DISABLE_COPY(IBenchmarkDescription)
    DISABLE_MOVE(IBenchmarkDescription)

public:
    /**
     * @brief Provides configuration for a benchmark creation.
     */
    struct BenchmarkConfig
    {
        /**
         * @brief Random seed used to generate synthetic data as input for benchmarks.
         */
        std::uint64_t random_seed;
        /**
         * @brief Default minimum test time for latency tests in milliseconds.
         * @details This will be the default time used when backends specify 0
         * for their minimum test time.
         */
        std::uint64_t default_min_test_time_ms;
        /**
         * @brief Default sample size to use for offline category.
         * @details This will be the default sample size used when backends specify 0
         * for the sample size of an operation parameter that can have variable sample size.
         * If this value is also 0, the sample size used will be the default value as
         * stated in the workload specification.
         */
        std::uint64_t default_sample_size;
    };
    /**
     * @brief Contains fields that describe a benchmark.
     * @details This structure is filled out by
     * IBenchmarkDescription::matchBenchmarkDescriptor() on a successful match.
     */
    struct Description
    {
        /**
         * @brief Human-readable friendly name of the benchmark workload.
         */
        std::string workload_name;
        /**
         * @brief CSV formatted header for this benchmark. This will be the header
         * pre-pended to the report containing the benchmark results.
         */
        std::string header;
        /**
         * @brief A string uniquely representing the benchmark descriptor that
         * can be used as a relative directory path. This may be several directories deep.
         */
        std::string path;
    };

    /**
     * @brief Token returned by a successful call to matchBenchmarkDescriptor().
     * @details Use the token returned by matchBenchmarkDescriptor() to instantiate
     * the actual benchmark.
     */
    class DescriptionToken
    {
    public:
        class FriendKeyCreation
        {
        public:
            friend class IBenchmarkDescription;

        private:
            FriendKeyCreation() {}
        };
        class FriendKeyAccess
        {
        public:
            friend class PartialBenchmark;

        private:
            FriendKeyAccess() {}
        };
        typedef std::shared_ptr<DescriptionToken> Ptr;

    public:
        DescriptionToken(const IBenchmarkDescription *p_caller,
                         const BenchmarkConfig &default_config,
                         const hebench::APIBridge::Handle &h_desc,
                         const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                         const std::vector<hebench::APIBridge::WorkloadParam> &w_params,
                         const FriendKeyCreation &) :
            m_p_caller(p_caller),
            m_h_desc(h_desc), m_bench_desc(bench_desc), m_w_params(w_params), m_config(default_config)
        {
        }

        /**
         * @brief Text description of a matched benchmark.
         */
        IBenchmarkDescription::Description description;

        const IBenchmarkDescription::BenchmarkConfig &getBenchmarkConfiguration(const void *p_caller) const
        {
            if (p_caller != m_p_caller)
                throw std::invalid_argument("Invalid calling object. This token can only be used by object that created it.");
            return m_config;
        }
        const hebench::APIBridge::Handle &getDescriptorHandle(const void *p_caller) const
        {
            if (p_caller != m_p_caller)
                throw std::invalid_argument("Invalid calling object. This token can only be used by object that created it.");
            return m_h_desc;
        }
        const hebench::APIBridge::BenchmarkDescriptor &getDescriptor(const void *p_caller) const
        {
            if (p_caller != m_p_caller)
                throw std::invalid_argument("Invalid calling object. This token can only be used by object that created it.");
            return m_bench_desc;
        }
        const std::vector<hebench::APIBridge::WorkloadParam> &getWorkloadParams(const void *p_caller) const
        {
            if (p_caller != m_p_caller)
                throw std::invalid_argument("Invalid calling object. This token can only be used by object that created it.");
            return m_w_params;
        }
        const void *getCaller(const FriendKeyAccess &) const { return m_p_caller; }

    private:
        const void *m_p_caller;
        hebench::APIBridge::Handle m_h_desc;
        hebench::APIBridge::BenchmarkDescriptor m_bench_desc;
        std::vector<hebench::APIBridge::WorkloadParam> m_w_params;
        BenchmarkConfig m_config;
    };

public:
    IBenchmarkDescription()          = default;
    virtual ~IBenchmarkDescription() = default;

    /**
     * @brief Determines if the represented benchmark can perform the workload described by
     * a specified HEBench benchmark descriptor and the workload parameters.
     * @param[out] description Structure to receive the text description of
     * the matched benchmark. Fields will not be valid if no match is found.
     * @param[in] engine Engine requesting the matching.
     * @param[in] h_desc Handle to descriptor to which compare.
     * @param[in] w_params Arguments for the workload parameters.
     * @returns A valid token representing the matched benchmark if the benchmark corresponding
     * to this `IBenchmarkDescription` object is compatible with the specified \p h_desc
     * and can perform the described workload.
     * @returns `null` otherwise.
     * @details
     * The token returned by this method can be passed to createBenchmark() to instantiate
     * the actual benchmark.
     *
     * This method is used by `BenchmarkFactory::createBenchmark()` to select
     * the appropriate benchmark to create based on the descriptor and the
     * workload parameters.
     */
    virtual DescriptionToken::Ptr matchBenchmarkDescriptor(const Engine &engine,
                                                           const IBenchmarkDescription::BenchmarkConfig &bench_config,
                                                           const hebench::APIBridge::Handle &h_desc,
                                                           const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const = 0;

    /**
     * @brief Creates the IBenchmark object that can perform the workload specified by the
     * HEBench benchmark descriptor.
     * @param[in] p_engine The engine creating the benchmark.
     * @param[in] description_token Description token representing the benchmark to create,
     * as matched by matchBenchmarkDescriptor().
     * @return Pointer to object of class derived from `IBenchmark` that can perform the
     * specified workload or `null` on unknown error.
     * @details
     * This method can throw instances of `std::exception` to communicate errors to caller
     * instead of returning `null`.
     *
     * This method constructs an object of type derived from IBenchmark, performs any
     * pending initialization on the fully constructed object, and returns a pointer to it.
     * The returned pointer must live until passed to method `destroyBenchmark()`.
     * @sa destroyBenchmark()
     */
    virtual PartialBenchmark *createBenchmark(std::shared_ptr<Engine> p_engine,
                                              DescriptionToken::Ptr p_description_token) = 0;
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
    DescriptionToken::Ptr createToken(const BenchmarkConfig &config,
                                      const hebench::APIBridge::Handle &h_desc,
                                      const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                      const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const;

private:
    DescriptionToken::FriendKeyCreation m_key_creation;
    //DescriptionToken::FriendKeyAccess m_key_access;
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
 * `PartialBenchmarkDescription::matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &)`,
 * and then, only if `true` is returned, it fills out the `Description` structure with
 * general information about the matched benchmark; `completeDescription()` is called
 * afterwards for clients to add or modify the general description.
 *
 * Clients need to implement:
 * - `IBenchmarkDescription::createBenchmark()`
 * - `IBenchmarkDescription::destroyBenchmark()`
 * - `PartialBenchmarkDescription::matchBenchmarkDescriptor()`
 * - `PartialBenchmarkDescription::completeDescription()`
 */
class PartialBenchmarkDescription : public IBenchmarkDescription
{
public:
    DISABLE_COPY(PartialBenchmarkDescription)
    DISABLE_MOVE(PartialBenchmarkDescription)
private:
    IL_DECLARE_CLASS_NAME(PartialBenchmarkDescription)

public:
    /**
     * @brief Extracts the batch sizes for a workload from a specified HEBench API
     * benchmark descriptor.
     * @param[out] sample_sizes Array with \p param_count elements to receive the sample
     * sizes for each operation parameter.
     * @param[in] param_count Number of parameters for the workload operation.
     * @param[in] default_batch_size If a batch size is set to 0 in the descriptor,
     * this value will be used as default instead.
     * @param[in] bench_desc HEBench API benchmark descriptor from which to extract the
     * workload batch sizes.
     * @return The batch size for the result of the operation.
     */
    static std::uint64_t computeSampleSizes(std::uint64_t *sample_sizes,
                                            std::size_t param_count,
                                            std::uint64_t default_sample_size,
                                            const hebench::APIBridge::BenchmarkDescriptor &bench_desc);

public:
    PartialBenchmarkDescription();
    ~PartialBenchmarkDescription() override;

    /**
     * @brief Implementation of IBenchmarkDescription::matchBenchmarkDescriptor().
     * @details Clients should not override this method.
     *
     * This method calls the internal overload
     * PartialBenchmarkDescription::matchBenchmarkDescriptor()
     * which clients must implement. If this overload returns a non-empty string, then method
     * PartialBenchmarkDescription::completeDescription() is called with a general description
     * of the matched benchmark already filled out to complete the description with
     * any extra information specific to the workload. It is recommended that clients
     * override completeDescription() to add extra information on top of the default
     * description.
     */
    DescriptionToken::Ptr matchBenchmarkDescriptor(const Engine &engine,
                                                   const BenchmarkConfig &bench_config,
                                                   const hebench::APIBridge::Handle &h_desc,
                                                   const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const override;

protected:
    static std::unordered_set<std::size_t> getCipherParamPositions(std::uint32_t cipher_param_mask);
    static std::string getCategoryName(hebench::APIBridge::Category category);
    static std::string getDataTypeName(hebench::APIBridge::DataType data_type);

protected:
    /**
     * @brief Determines if the represented benchmark can perform the workload described by
     * a specified HEBench benchmark descriptor and workload parameters.
     * @param[in] bench_desc Descriptor to which compare.
     * @param[in] w_params Arguments for the workload parameters.
     * @return Human-readable friendly name for the matched workload to be used for its
     * description on report or empty string if no match was found.
     * @details
     * This method is used by `BenchmarkFactory::createBenchmark()` to select
     * the appropriate benchmark to create based on the descriptor.
     */
    virtual std::string matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                                 const std::vector<hebench::APIBridge::WorkloadParam> &w_params) const = 0;

    /**
     * @brief Completes filling out the Description for the specified descriptor.
     * @param[in,out] description Structure to contain the text description of
     * the matched benchmark. This is pre-filled with general benchmark description.
     * The purpose of this call is to add or modify the description with any extra
     * data that is workload specific.
     * @param[in] engine Engine that requested the matching.
     * @param[in] bench_desc HEBench API descriptor to be described.
     * @param[in] w_params Arguments for the workload parameters.
     * @throws std::invalid_argument if fields in \p bench_desc are unsuported or invalid.
     * @details This method will only be called if
     * `PartialBenchmarkDescription::matchBenchmarkDescriptor(const hebench::APIBridge::BenchmarkDescriptor &)`
     * returns `true`.
     *
     * This method receives a pre-initialized and pre-filled Description structure in
     * \p description. All fields will be filled out with the general description of
     * the benchmark based on the data from \p bench_desc. The purpose of this method is
     * to complete the description of the CSV header and/or workload name with any
     * information specific to this workload.
     *
     * If no other information needs to be added, this implementation can return immediately.
     *
     * An example header pre-filled and received by this method has the form:
     *
     * @code
     * Specifications,
     * , Encryption,
     * , , Scheme, CKKS
     * , , Security, 128 bits
     * , , Poly mod degree, 8192          <--- This is example extra description returned by
     * , , Primes, 3                      <--- backend
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
     * Clients overriding this method can append extra description to the header after
     * this point or just create a new header entirely. Workload name and path are
     * also pre-filled out.
     */
    virtual void completeDescription(const Engine &engine,
                                     DescriptionToken::Ptr pre_token) const = 0;

private:
    void describe(const Engine &engine,
                  DescriptionToken::Ptr pre_token) const;
};

/**
 * @brief Interface for benchmarks.
 * @details To implement this interface and create workloads, it is easier to extend
 * directly from the workload category benchmark classes BenchmarkLatency, BenchmarkOffline,
 * etc. If more control or specific processing other than the generic category execution
 * is needed, extending from PartialBenchmark offers more flexibility.
 *
 * <b>Execution flow:</b>
 *
 * During execution of a single benchmark, Test Harness will call `IBenchmark::create()`
 * with parameters to select and create the right benchmark.
 *
 * During benchmark creation, dataset loading or generation compatible with the benchmark
 * is expected to occur.
 *
 * From the benchmark object created, Test Harness calls obj->run(TimingReport &).
 *
 * After the benchmark run completes without exceptions, the TimingReport gets formated,
 * summarized and writen to files in the location specified by obj->getDescriptorPath().
 * If this location is relative, a root location will be prepended to it as specified by
 * process command line arguments. This path does not need to exist (it will be created
 * automatically or Test Harness will terminate if an error occurs).
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
    //virtual std::string getExtraDescription() const          = 0;

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
    const hebench::APIBridge::BenchmarkDescriptor &getDescriptor() const { return m_descriptor; }
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
     * @param[in] description The text description generated for this benchmark.
     * @details This method is provided to allow clients to perform polymorphic
     * initialization outside of the class contructor before the backend benchmark
     * is initialized.
     *
     * This method is called automatically during creation and initialization
     * of the benchmark.
     */
    virtual void init(const IBenchmarkDescription::Description &description) = 0;
    /**
     * @brief Initializes backend benchmark.
     * @details HEBench API calls are performed here in order to initialize the
     * benchmark corresponding to the description token used during construction.
     */
    void initBackend(hebench::Utilities::TimingReportEx &out_report, const FriendPrivateKey &);
    /**
     * @brief Called automatically during initialization after the backend has
     * been initialized.
     * @details This method is provided to allow clients to perform polymorphic
     * initialization outside of the class contructor after the backend benchmark
     * has been initialized, if needed.
     *
     * Derived classes MUST call base implementation as first step when overriding,
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
     * PartialBenchmark has not been called (for those, then this method will fail
     * and the exception will be thrown.
     * @sa init(), initBackend(), postInit()
     */
    void checkInitializationState(const FriendPrivateKey &) const;

protected:
    /**
     * @brief Allows read-only access to this benchmark descriptor.
     */
    const hebench::APIBridge::BenchmarkDescriptor &m_descriptor;
    /**
     * @brief Allows read-only access to the workloads parameters
     * used for this benchmark.
     */
    const std::vector<hebench::APIBridge::WorkloadParam> &m_params;
    /**
     * @brief Allows read-only access to the benchmark configuration data.
     */
    const IBenchmarkDescription::BenchmarkConfig &m_benchmark_configuration;

    PartialBenchmark(std::shared_ptr<Engine> p_engine,
                     const IBenchmarkDescription::DescriptionToken &description_token);

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
    void internalInit(const IBenchmarkDescription::DescriptionToken &description_token);

    IBenchmarkDescription::DescriptionToken::FriendKeyAccess m_key_adk; // friend key to access description token
    std::shared_ptr<Engine> m_p_engine;
    hebench::APIBridge::Handle m_handle;
    hebench::APIBridge::Handle m_h_descriptor;
    hebench::APIBridge::BenchmarkDescriptor m_benchmark_descriptor;
    std::vector<hebench::APIBridge::WorkloadParam> m_workload_params;
    IBenchmarkDescription::BenchmarkConfig m_bench_config;
    std::uint32_t m_current_event_id;
    bool m_b_constructed;
    bool m_b_initialized;
};

} // namespace TestHarness
} // namespace hebench

#endif // defined _HEBench_Harness_IBenchmark_H_0596d40a3cce4b108a81595c50eb286d
