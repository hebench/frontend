
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <array>

#include "hebench/api_bridge/cpp/hebench.hpp"

class TutorialEngine;

class TutorialEltwiseAddBenchmarkDescription : public hebench::cpp::BenchmarkDescription
{
public:
    HEBERROR_DECLARE_CLASS_NAME(TutorialEltwiseAddBenchmarkDescription)

public:
    // This workload (EltwiseAdd) requires only 1 parameter
    static constexpr std::uint64_t NumWorkloadParams = 1;

    // HE specific parameters
    static constexpr std::size_t PolyModulusDegree    = 8192;
    static constexpr std::size_t NumCoefficientModuli = 2;
    static constexpr int ScaleExponent                = 40;

    TutorialEltwiseAddBenchmarkDescription();
    ~TutorialEltwiseAddBenchmarkDescription() override;

    std::string getBenchmarkDescription(const hebench::APIBridge::WorkloadParams *p_w_params) const override;

    hebench::cpp::BaseBenchmark *createBenchmark(hebench::cpp::BaseEngine &engine,
                                                 const hebench::APIBridge::WorkloadParams *p_params) override;
    void destroyBenchmark(hebench::cpp::BaseBenchmark *p_bench) override;
};

class TutorialEltwiseAddBenchmark : public hebench::cpp::BaseBenchmark
{
public:
    HEBERROR_DECLARE_CLASS_NAME(TutorialEltwiseAddBenchmark)

public:
    static constexpr std::int64_t tag = 0x1;

    TutorialEltwiseAddBenchmark(TutorialEngine &engine,
                                const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                const hebench::APIBridge::WorkloadParams &bench_params);
    ~TutorialEltwiseAddBenchmark() override;

    hebench::APIBridge::Handle encode(const hebench::APIBridge::PackedData *p_parameters) override;
    void decode(hebench::APIBridge::Handle encoded_data, hebench::APIBridge::PackedData *p_native) override;
    hebench::APIBridge::Handle encrypt(hebench::APIBridge::Handle encoded_data) override;
    hebench::APIBridge::Handle decrypt(hebench::APIBridge::Handle encrypted_data) override;

    hebench::APIBridge::Handle load(const hebench::APIBridge::Handle *p_local_data, std::uint64_t count) override;
    void store(hebench::APIBridge::Handle remote_data,
               hebench::APIBridge::Handle *p_local_data, std::uint64_t count) override;

    hebench::APIBridge::Handle operate(hebench::APIBridge::Handle h_remote_packed,
                                       const hebench::APIBridge::ParameterIndexer *p_param_indexers) override;

    std::int64_t classTag() const override { return BaseBenchmark::classTag() | TutorialEltwiseAddBenchmark::tag; }

private:
    static constexpr std::uint64_t ParametersCount       = 2; // number of parameters for this operation
    static constexpr std::uint64_t ResultComponentsCount = 1; // number of components of result for this operation

    // used to bundle a collection of operation parameters
    struct InternalParams
    {
    public:
        static constexpr std::int64_t tagPlaintext  = 0x10;
        static constexpr std::int64_t tagCiphertext = 0x20;

        std::vector<std::shared_ptr<void>> samples;
        std::int64_t tag;
        std::uint64_t param_position;
    };
};
