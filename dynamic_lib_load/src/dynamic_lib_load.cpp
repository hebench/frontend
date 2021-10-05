// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifdef __linux__
#include <dlfcn.h>
#else
#error("Source file only supported in LINUX!")
#endif

#include "dynamic_lib_load.h"

namespace hebench {
namespace APIBridge {

typedef ErrorCode (*DestroyHandle)(Handle h);

typedef ErrorCode (*InitEngine)(Handle *h_engine);

typedef ErrorCode (*SubscribeBenchmarksCount)(Handle h_engine, std::uint64_t *p_count);

typedef ErrorCode (*SubscribeBenchmarks)(Handle h_engine, Handle *p_bench_descs);

typedef ErrorCode (*GetWorkloadParamsDetails)(Handle h_engine,
                                              Handle h_bench_desc,
                                              std::uint64_t *p_param_count,
                                              std::uint64_t *p_default_count);

typedef ErrorCode (*DescribeBenchmark)(Handle h_engine,
                                       Handle h_bench_desc,
                                       BenchmarkDescriptor *p_bench_desc,
                                       WorkloadParams *p_default_params);

typedef ErrorCode (*InitBenchmark)(Handle h_engine,
                                   Handle h_bench_desc,
                                   const WorkloadParams *p_params,
                                   Handle *h_benchmark);

typedef ErrorCode (*Encode)(Handle h_benchmark,
                            const PackedData *p_parameters,
                            Handle *h_plaintext);

typedef ErrorCode (*Decode)(Handle h_benchmark,
                            Handle h_plaintext,
                            PackedData *p_native);

typedef ErrorCode (*Encrypt)(Handle h_benchmark,
                             Handle h_plaintext,
                             Handle *h_ciphertext);

typedef ErrorCode (*Decrypt)(Handle h_benchmark,
                             Handle h_ciphertext,
                             Handle *h_plaintext);

typedef ErrorCode (*Load)(Handle h_benchmark,
                          const Handle *h_local_packed_params,
                          std::uint64_t local_count,
                          Handle *h_remote);

typedef ErrorCode (*Store)(Handle h_benchmark,
                           Handle h_remote,
                           Handle *h_local_packed_params,
                           std::uint64_t local_count);

typedef ErrorCode (*Operate)(Handle h_benchmark,
                             Handle h_remote_packed_params,
                             const ParameterIndexer *p_param_indexers,
                             Handle *h_remote_output);

typedef std::uint64_t (*GetSchemeName)(Handle h_engine, Scheme s, char *p_name, std::uint64_t size);
typedef std::uint64_t (*GetSchemeSecurityName)(Handle h_engine, Scheme s, Security sec,
                                               char *p_name, std::uint64_t size);

typedef std::uint64_t (*GetBenchmarkDescriptionEx)(Handle h_engine,
                                                   Handle h_bench_desc,
                                                   const hebench::APIBridge::WorkloadParams *p_w_params,
                                                   char *p_description, std::uint64_t size);

typedef std::uint64_t (*GetErrorDescription)(ErrorCode code, char *p_description, std::uint64_t size);

typedef std::uint64_t (*GetLastErrorDescription)(Handle h_engine, char *p_description, std::uint64_t size);

/**
 * @brief Holds function pointers to each method in the API Bridge with external linkage
 * @details Each data member contains the function pointer that one would expect based on
 * the name of the variable. e.g. destroyHandle points to the destroyHandle() for the
 * loaded backend.
 */
struct ExternFunctions
{
    DestroyHandle destroyHandle;
    InitEngine initEngine;
    SubscribeBenchmarksCount subscribeBenchmarksCount;
    SubscribeBenchmarks subscribeBenchmarks;
    GetWorkloadParamsDetails getWorkloadParamsDetails;
    DescribeBenchmark describeBenchmark;
    InitBenchmark initBenchmark;
    Encode encode;
    Decode decode;
    Encrypt encrypt;
    Decrypt decrypt;
    Load load;
    Store store;
    Operate operate;
    GetSchemeName getSchemeName;
    GetSchemeSecurityName getSchemeSecurityName;
    GetBenchmarkDescriptionEx getBenchmarkDescriptionEx;
    GetErrorDescription getErrorDescription;
    GetLastErrorDescription getLastErrorDescription;
};

struct DynamicLib
{
    std::string path;
    void *handle;

    DynamicLib(std::string p) :
        path(p), handle(nullptr)
    {
        handle = dlopen(path.c_str(), RTLD_NOW);
        if (!handle)
            throw std::runtime_error(dlerror());
        dlerror(); // reset
    }

    ~DynamicLib()
    {
        if (handle)
            dlclose(handle);
    }
};

ExternFunctions DynamicLibLoad::m_functions;
DynamicLib *DynamicLibLoad::m_lib = nullptr;

void DynamicLibLoad::loadLibrary(const std::string &path)
{
    std::cout << "[ Info    ] Loading Backend Library..." << std::endl;
    m_lib = new DynamicLib(path);

    std::cout << "[ Info    ] Finding Backend Symbols in Memory..." << std::endl;
    m_functions.destroyHandle             = (DestroyHandle)loadSymbol(m_lib->handle, "destroyHandle");
    m_functions.initEngine                = (InitEngine)loadSymbol(m_lib->handle, "initEngine");
    m_functions.subscribeBenchmarksCount  = (SubscribeBenchmarksCount)loadSymbol(m_lib->handle, "subscribeBenchmarksCount");
    m_functions.subscribeBenchmarks       = (SubscribeBenchmarks)loadSymbol(m_lib->handle, "subscribeBenchmarks");
    m_functions.getWorkloadParamsDetails  = (GetWorkloadParamsDetails)loadSymbol(m_lib->handle, "getWorkloadParamsDetails");
    m_functions.describeBenchmark         = (DescribeBenchmark)loadSymbol(m_lib->handle, "describeBenchmark");
    m_functions.initBenchmark             = (InitBenchmark)loadSymbol(m_lib->handle, "initBenchmark");
    m_functions.encode                    = (Encode)loadSymbol(m_lib->handle, "encode");
    m_functions.decode                    = (Decode)loadSymbol(m_lib->handle, "decode");
    m_functions.encrypt                   = (Encrypt)loadSymbol(m_lib->handle, "encrypt");
    m_functions.decrypt                   = (Decrypt)loadSymbol(m_lib->handle, "decrypt");
    m_functions.load                      = (Load)loadSymbol(m_lib->handle, "load");
    m_functions.store                     = (Store)loadSymbol(m_lib->handle, "store");
    m_functions.operate                   = (Operate)loadSymbol(m_lib->handle, "operate");
    m_functions.getSchemeName             = (GetSchemeName)loadSymbol(m_lib->handle, "getSchemeName");
    m_functions.getSchemeSecurityName     = (GetSchemeSecurityName)loadSymbol(m_lib->handle, "getSchemeSecurityName");
    m_functions.getBenchmarkDescriptionEx = (GetBenchmarkDescriptionEx)loadSymbol(m_lib->handle, "getBenchmarkDescriptionEx");
    m_functions.getErrorDescription       = (GetErrorDescription)loadSymbol(m_lib->handle, "getErrorDescription");
    m_functions.getLastErrorDescription   = (GetLastErrorDescription)loadSymbol(m_lib->handle, "getLastErrorDescription");
    std::cout << "[    DONE ] " << std::endl;
}

void DynamicLibLoad::unloadLibrary()
{
    if (m_lib)
    {
        delete m_lib;
        m_lib = nullptr;
    } // end if
}

void *DynamicLibLoad::loadSymbol(void *handle, const std::string &name)
{
    void *fptr = dlsym(handle, name.c_str());
    if (!fptr)
        throw std::runtime_error(dlerror());
    dlerror(); // reset
    return fptr;
}

ErrorCode DynamicLibLoad::destroyHandle(Handle h)
{
    return m_functions.destroyHandle(h);
}

ErrorCode DynamicLibLoad::initEngine(Handle *h_engine)
{
    return m_functions.initEngine(h_engine);
}

ErrorCode DynamicLibLoad::subscribeBenchmarksCount(Handle h_engine, std::uint64_t *p_count)
{
    return m_functions.subscribeBenchmarksCount(h_engine, p_count);
}

ErrorCode DynamicLibLoad::subscribeBenchmarks(Handle h_engine, Handle *p_h_bench_descs)
{
    return m_functions.subscribeBenchmarks(h_engine, p_h_bench_descs);
}

ErrorCode DynamicLibLoad::getWorkloadParamsDetails(Handle h_engine, Handle h_bench_desc, std::uint64_t *p_param_count, std::uint64_t *p_default_count)
{
    return m_functions.getWorkloadParamsDetails(h_engine, h_bench_desc, p_param_count, p_default_count);
}

ErrorCode DynamicLibLoad::describeBenchmark(Handle h_engine, Handle h_bench_desc, BenchmarkDescriptor *p_bench_desc, WorkloadParams *p_default_params)
{
    return m_functions.describeBenchmark(h_engine, h_bench_desc, p_bench_desc, p_default_params);
}

ErrorCode DynamicLibLoad::initBenchmark(Handle h_engine, Handle h_bench_desc, const WorkloadParams *p_params, Handle *h_benchmark)
{
    return m_functions.initBenchmark(h_engine, h_bench_desc, p_params, h_benchmark);
}

ErrorCode DynamicLibLoad::encode(Handle h_benchmark, const PackedData *p_parameters, Handle *h_plaintext)
{
    return m_functions.encode(h_benchmark, p_parameters, h_plaintext);
}

ErrorCode DynamicLibLoad::decode(Handle h_benchmark, Handle h_plaintext, PackedData *p_native)
{
    return m_functions.decode(h_benchmark, h_plaintext, p_native);
}

ErrorCode DynamicLibLoad::encrypt(Handle h_benchmark, Handle h_plaintext, Handle *h_ciphertext)
{
    return m_functions.encrypt(h_benchmark, h_plaintext, h_ciphertext);
}

ErrorCode DynamicLibLoad::decrypt(Handle h_benchmark, Handle h_ciphertext, Handle *h_plaintext)
{
    return m_functions.decrypt(h_benchmark, h_ciphertext, h_plaintext);
}

ErrorCode DynamicLibLoad::load(Handle h_benchmark,
                               const Handle *h_local_packed_params, std::uint64_t local_count,
                               Handle *h_remote_packed_params)
{
    return m_functions.load(h_benchmark, h_local_packed_params, local_count, h_remote_packed_params);
}

ErrorCode DynamicLibLoad::store(Handle h_benchmark,
                                Handle h_remote,
                                Handle *h_local_packed_params, std::uint64_t local_count)
{
    return m_functions.store(h_benchmark, h_remote, h_local_packed_params, local_count);
}

ErrorCode DynamicLibLoad::operate(Handle h_benchmark,
                                  Handle h_remote_packed_params,
                                  const ParameterIndexer *p_param_indexers,
                                  Handle *h_remote_output)
{
    return m_functions.operate(h_benchmark, h_remote_packed_params, p_param_indexers, h_remote_output);
}

std::uint64_t DynamicLibLoad::getSchemeName(Handle h_engine, Scheme s, char *p_name, std::uint64_t size)
{
    return m_functions.getSchemeName(h_engine, s, p_name, size);
}

std::uint64_t DynamicLibLoad::getSchemeSecurityName(Handle h_engine, Scheme s, Security sec,
                                                    char *p_name, std::uint64_t size)
{
    return m_functions.getSchemeSecurityName(h_engine, s, sec, p_name, size);
}

std::uint64_t DynamicLibLoad::getBenchmarkDescriptionEx(Handle h_engine,
                                                        Handle h_bench_desc,
                                                        const hebench::APIBridge::WorkloadParams *p_w_params,
                                                        char *p_description, std::uint64_t size)
{
    return m_functions.getBenchmarkDescriptionEx(h_engine, h_bench_desc, p_w_params, p_description, size);
}

std::uint64_t DynamicLibLoad::getErrorDescription(ErrorCode code, char *p_description, std::uint64_t size)
{
    return m_functions.getErrorDescription(code, p_description, size);
}

std::uint64_t DynamicLibLoad::getLastErrorDescription(Handle h_engine, char *p_description, std::uint64_t size)
{
    return m_functions.getLastErrorDescription(h_engine, p_description, size);
}

} // namespace APIBridge
} // namespace hebench
