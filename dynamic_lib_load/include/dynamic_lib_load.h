// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef _HEBench_Dynamic_Lib_Load_H_7e5fa8c2415240ea93eff148ed73539b
#define _HEBench_Dynamic_Lib_Load_H_7e5fa8c2415240ea93eff148ed73539b

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include "hebench/api_bridge/api.h"

namespace hebench {
namespace APIBridge {

struct ExternFunctions;
struct DynamicLib;

/**
 * @brief "Static class" re-implementing API Bridge methods to direct calls to correct backend
 * @details If wanting to debug the backend being loaded, you will need to tell the debugger
 * where the backend library is located. How this is done depends on the debugger/IDE being used.
 * For example, in QTCreator, you must add the following line to the GDB startup commands in
 * options (Tools > Options > Debugger > GDB > Additional Startup Commands):
 * "set solib-search-path /path1/to/backend/library/directory:/path2/to/backend/library/directory"
 */
class DynamicLibLoad
{
public:
    /**
     * @brief Loads the pre-built backend library at the specified path
     * @param[in] path The filepath to the pre-built backend library
     * @exception std::runtime_error Thrown when dlopen fails to load the
     * library located at \p path ususally due to improper ELF header or malformed
     * binary.
     * @exception std::runtime_error Thrown if the symbol specified cannot be found
     * for the loaded library (or any libraries loaded by dlopen) in memory.
     * @details If no exceptions were thrown, the backend loading was a success.
     * The test harness may then simply continue execution utilizing the API Bridge
     * methods freely. The calls will divert to the specified loaded backend library.
     */
    static void loadLibrary(const std::string &path);
    static void unloadLibrary();

    static ErrorCode destroyHandle(Handle h);
    static ErrorCode initEngine(Handle *h_engine);
    static ErrorCode subscribeBenchmarksCount(Handle h_engine, std::uint64_t *p_count);
    static ErrorCode subscribeBenchmarks(Handle h_engine, Handle *p_h_bench_descs);
    static ErrorCode getWorkloadParamsDetails(Handle h_engine,
                                              Handle h_bench_desc,
                                              std::uint64_t *p_param_count,
                                              std::uint64_t *p_default_count);
    static ErrorCode describeBenchmark(Handle h_engine,
                                       Handle h_bench_desc,
                                       BenchmarkDescriptor *p_bench_desc,
                                       WorkloadParams *p_default_params);
    static ErrorCode createBenchmark(Handle h_engine,
                                     Handle h_bench_desc,
                                     const WorkloadParams *p_params,
                                     Handle *h_benchmark);
    static ErrorCode initBenchmark(Handle h_benchmark,
                                   const BenchmarkDescriptor *p_concrete_desc);
    static ErrorCode encode(Handle h_benchmark,
                            const DataPackCollection *p_parameters,
                            Handle *h_plaintext);
    static ErrorCode decode(Handle h_benchmark,
                            Handle h_plaintext,
                            DataPackCollection *p_native);
    static ErrorCode encrypt(Handle h_benchmark,
                             Handle h_plaintext,
                             Handle *h_ciphertext);
    static ErrorCode decrypt(Handle h_benchmark,
                             Handle h_ciphertext,
                             Handle *h_plaintext);
    static ErrorCode load(Handle h_benchmark,
                          const Handle *h_local_packed_params,
                          std::uint64_t local_count,
                          Handle *h_remote);
    static ErrorCode store(Handle h_benchmark,
                           Handle h_remote,
                           Handle *h_local_packed_params,
                           std::uint64_t local_count);
    static ErrorCode operate(Handle h_benchmark,
                             Handle h_remote_packed_params,
                             const ParameterIndexer *p_param_indexers,
                             Handle *h_remote_output);
    static std::uint64_t getSchemeName(Handle h_engine, Scheme s, char *p_name, std::uint64_t size);
    static std::uint64_t getSchemeSecurityName(Handle h_engine, Scheme s, Security sec,
                                               char *p_name, std::uint64_t size);
    static std::uint64_t getBenchmarkDescriptionEx(Handle h_engine,
                                                   Handle h_bench_desc,
                                                   const hebench::APIBridge::WorkloadParams *p_w_params,
                                                   char *p_description, std::uint64_t size);
    static std::uint64_t getErrorDescription(ErrorCode code, char *p_description, std::uint64_t size);
    static std::uint64_t getLastErrorDescription(Handle h_engine, char *p_description, std::uint64_t size);

private:
    /**
     * @brief Object allowing calling of dynamically loaded backends through form
     * m_functions.API_Bridge_Function(params)
     */
    static ExternFunctions m_functions;
    static DynamicLib *m_lib;

    /**
     * @brief Constructor made private to avoid any unnecessary instance creation
     */
    DynamicLibLoad() {}
    static void *loadSymbol(void *handle, const std::string &name);
};
} // namespace APIBridge
} // namespace hebench

#endif // define _HEBench_Dynamic_Lib_Load_H_7e5fa8c2415240ea93eff148ed73539b
