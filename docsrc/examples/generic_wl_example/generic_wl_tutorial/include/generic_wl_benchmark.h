
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

/// @cond

#include <array>

#include "hebench/api_bridge/cpp/hebench.hpp"

class ExampleEngine;

class ExampleBenchmarkDescription : public hebench::cpp::BenchmarkDescription
{
public:
    HEBERROR_DECLARE_CLASS_NAME(ExampleBenchmarkDescription)

public:
    // This workload requires 7 workload parameters
    static constexpr std::uint64_t NumWorkloadParams = 7;
    ExampleBenchmarkDescription(hebench::APIBridge::Category category);
    ~ExampleBenchmarkDescription() override;

    hebench::cpp::BaseBenchmark *createBenchmark(hebench::cpp::BaseEngine &engine,
                                                 const hebench::APIBridge::WorkloadParams *p_params) override;
    void destroyBenchmark(hebench::cpp::BaseBenchmark *p_bench) override;
};

class ExampleBenchmark : public hebench::cpp::BaseBenchmark
{
public:
    HEBERROR_DECLARE_CLASS_NAME(ExampleBenchmark)

public:
    static constexpr std::int64_t tag = 0x1;

    ExampleBenchmark(ExampleEngine &engine,
                     const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                     const hebench::APIBridge::WorkloadParams &bench_params);
    ~ExampleBenchmark() override;

    hebench::APIBridge::Handle encode(const hebench::APIBridge::PackedData *p_parameters) override;
    void decode(hebench::APIBridge::Handle encoded_data, hebench::APIBridge::PackedData *p_native) override;
    hebench::APIBridge::Handle encrypt(hebench::APIBridge::Handle encoded_data) override;
    hebench::APIBridge::Handle decrypt(hebench::APIBridge::Handle encrypted_data) override;

    hebench::APIBridge::Handle load(const hebench::APIBridge::Handle *p_local_data, std::uint64_t count) override;
    void store(hebench::APIBridge::Handle remote_data,
               hebench::APIBridge::Handle *p_local_data, std::uint64_t count) override;

    hebench::APIBridge::Handle operate(hebench::APIBridge::Handle h_remote_packed,
                                       const hebench::APIBridge::ParameterIndexer *p_param_indexers) override;

    std::int64_t classTag() const override { return BaseBenchmark::classTag() | ExampleBenchmark::tag; }

private:
    static constexpr std::uint64_t ParametersCount       = 2; // number of parameters for this operation
    static constexpr std::uint64_t ResultComponentsCount = 3; // number of components of result for this operation

    // InternalInputData[input_param_index][input_sample][element]
    typedef std::array<std::vector<std::vector<double>>, ParametersCount> InternalInputData;
    // InternalResultData[result_sample][result_component][element]
    typedef std::vector<std::array<std::vector<double>, ResultComponentsCount>> InternalResultData;
};

/// @endcond
