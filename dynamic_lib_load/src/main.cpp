// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "dynamic_lib_load.h"

namespace hebench {
namespace APIBridge {

ErrorCode destroyHandle(Handle h)
{
    return DynamicLibLoad::destroyHandle(h);
}

ErrorCode initEngine(Handle *h_engine)
{
    return DynamicLibLoad::initEngine(h_engine);
}

ErrorCode subscribeBenchmarksCount(Handle h_engine, std::uint64_t *p_count)
{
    return DynamicLibLoad::subscribeBenchmarksCount(h_engine, p_count);
}

ErrorCode subscribeBenchmarks(Handle h_engine, Handle *p_h_bench_descs)
{
    return DynamicLibLoad::subscribeBenchmarks(h_engine, p_h_bench_descs);
}

ErrorCode getWorkloadParamsDetails(Handle h_engine, Handle h_bench_desc, std::uint64_t *p_param_count, std::uint64_t *p_default_count)
{
    return DynamicLibLoad::getWorkloadParamsDetails(h_engine, h_bench_desc, p_param_count, p_default_count);
}

ErrorCode describeBenchmark(Handle h_engine, Handle h_bench_desc, BenchmarkDescriptor *p_bench_desc, WorkloadParams *p_default_params)
{
    return DynamicLibLoad::describeBenchmark(h_engine, h_bench_desc, p_bench_desc, p_default_params);
}

ErrorCode initBenchmark(Handle h_engine, Handle h_bench_desc, const WorkloadParams *p_params, Handle *h_benchmark)
{
    return DynamicLibLoad::initBenchmark(h_engine, h_bench_desc, p_params, h_benchmark);
}

ErrorCode encode(Handle h_benchmark, const PackedData *p_parameters, Handle *h_plaintext)
{
    return DynamicLibLoad::encode(h_benchmark, p_parameters, h_plaintext);
}

ErrorCode decode(Handle h_benchmark, Handle h_plaintext, PackedData *p_native)
{
    return DynamicLibLoad::decode(h_benchmark, h_plaintext, p_native);
}

ErrorCode encrypt(Handle h_benchmark, Handle h_plaintext, Handle *h_ciphertext)
{
    return DynamicLibLoad::encrypt(h_benchmark, h_plaintext, h_ciphertext);
}

ErrorCode decrypt(Handle h_benchmark, Handle h_ciphertext, Handle *h_plaintext)
{
    return DynamicLibLoad::decrypt(h_benchmark, h_ciphertext, h_plaintext);
}

ErrorCode load(Handle h_benchmark,
               const Handle *h_local_packed_params, std::uint64_t local_count,
               Handle *h_remote_packed_params)
{
    return DynamicLibLoad::load(h_benchmark, h_local_packed_params, local_count, h_remote_packed_params);
}

ErrorCode store(Handle h_benchmark,
                Handle h_remote,
                Handle *h_local_packed_params, std::uint64_t local_count)
{
    return DynamicLibLoad::store(h_benchmark, h_remote, h_local_packed_params, local_count);
}

ErrorCode operate(Handle h_benchmark,
                  Handle h_remote_packed_params,
                  const ParameterIndexer *p_param_indexers,
                  Handle *h_remote_output)
{
    return DynamicLibLoad::operate(h_benchmark, h_remote_packed_params, p_param_indexers, h_remote_output);
}

std::uint64_t getSchemeName(Handle h_engine, Scheme s, char *p_name, std::uint64_t size)
{
    return DynamicLibLoad::getSchemeName(h_engine, s, p_name, size);
}

std::uint64_t getSchemeSecurityName(Handle h_engine, Scheme s, Security sec,
                                    char *p_name, std::uint64_t size)
{
    return DynamicLibLoad::getSchemeSecurityName(h_engine, s, sec, p_name, size);
}

std::uint64_t getBenchmarkDescriptionEx(Handle h_engine,
                                        Handle h_bench_desc,
                                        const hebench::APIBridge::WorkloadParams *p_w_params,
                                        char *p_description, std::uint64_t size)
{
    return DynamicLibLoad::getBenchmarkDescriptionEx(h_engine, h_bench_desc, p_w_params, p_description, size);
}

std::uint64_t getErrorDescription(ErrorCode code, char *p_description, std::uint64_t size)
{
    return DynamicLibLoad::getErrorDescription(code, p_description, size);
}

std::uint64_t getLastErrorDescription(Handle h_engine, char *p_description, std::uint64_t size)
{
    return DynamicLibLoad::getLastErrorDescription(h_engine, p_description, size);
}

} // namespace APIBridge
} // namespace hebench
