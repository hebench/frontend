
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

/// @cond

#include <array>

#include "hebench/api_bridge/cpp/hebench.hpp"
#include "seal/seal.h"

//! [benchmark description class]
class TutorialEltwiseAddBenchmarkDescription : public hebench::cpp::BenchmarkDescription
//! [benchmark description class]
{
public:
    HEBERROR_DECLARE_CLASS_NAME(TutorialEltwiseAddBenchmarkDescription)

public:
    // This workload (EltwiseAdd) requires only 1 parameter
    static constexpr std::uint64_t NumWorkloadParams = 1;

    // Operation specifics
    static constexpr std::uint64_t ParametersCount       = 2; // number of parameters for this operation
    static constexpr std::uint64_t ResultComponentsCount = 1; // number of components of result for this operation

    // HE specific parameters
    static constexpr std::size_t PolyModulusDegree    = 8192;
    static constexpr std::size_t NumCoefficientModuli = 1;

    TutorialEltwiseAddBenchmarkDescription();
    ~TutorialEltwiseAddBenchmarkDescription() override;

    std::string getBenchmarkDescription(const hebench::APIBridge::WorkloadParams *p_w_params) const override;
    hebench::cpp::BaseBenchmark *createBenchmark(hebench::cpp::BaseEngine &engine,
                                                 const hebench::APIBridge::WorkloadParams *p_params) override;
    void destroyBenchmark(hebench::cpp::BaseBenchmark *p_bench) override;
};

//! [benchmark class]
class TutorialEltwiseAddBenchmark : public hebench::cpp::BaseBenchmark
//! [benchmark class]
{
public:
    HEBERROR_DECLARE_CLASS_NAME(TutorialEltwiseAddBenchmark)

public:
    static constexpr std::int64_t tag = 0x1;

    TutorialEltwiseAddBenchmark(hebench::cpp::BaseEngine &engine,
                                const hebench::APIBridge::BenchmarkDescriptor &bench_desc,
                                const hebench::APIBridge::WorkloadParams &bench_params);
    ~TutorialEltwiseAddBenchmark() override;

    hebench::APIBridge::Handle encode(const hebench::APIBridge::DataPackCollection *p_parameters) override;
    void decode(hebench::APIBridge::Handle encoded_data, hebench::APIBridge::DataPackCollection *p_native) override;
    hebench::APIBridge::Handle encrypt(hebench::APIBridge::Handle encoded_data) override;
    hebench::APIBridge::Handle decrypt(hebench::APIBridge::Handle encrypted_data) override;

    hebench::APIBridge::Handle load(const hebench::APIBridge::Handle *p_local_data, std::uint64_t count) override;
    void store(hebench::APIBridge::Handle remote_data,
               hebench::APIBridge::Handle *p_local_data, std::uint64_t count) override;

    hebench::APIBridge::Handle operate(hebench::APIBridge::Handle h_remote_packed,
                                       const hebench::APIBridge::ParameterIndexer *p_param_indexers) override;

    std::int64_t classTag() const override { return BaseBenchmark::classTag() | TutorialEltwiseAddBenchmark::tag; }

private:
    //! [benchmark class internal_representation]
    struct InternalParamInfo
    {
    public:
        static constexpr std::int64_t tagPlaintext  = 0x10;
        static constexpr std::int64_t tagCiphertext = 0x20;

        std::uint64_t param_position;
        std::int64_t tag;
    };
    // used to bundle a collection of samples for an operation parameter
    template <class T>
    struct InternalParam : public InternalParamInfo
    {
    public:
        std::vector<T> samples;
    };
    //! [benchmark class internal_representation]

    //! [benchmark class original_workload]
    //! [benchmark class original_workload declaration]
    class Workload
    //! [benchmark class original_workload declaration]
    {
    public:
        Workload(std::size_t vector_size);

        std::vector<seal::Plaintext> encodeVector(const std::vector<std::vector<std::int64_t>> &vec);
        std::vector<seal::Plaintext> encodeVector(const std::vector<gsl::span<const std::int64_t>> &vec);
        std::vector<seal::Ciphertext> encryptVector(const std::vector<seal::Plaintext> &encoded_vec);
        std::vector<seal::Ciphertext> eltwiseadd(const std::vector<seal::Plaintext> &A,
                                                 const std::vector<seal::Ciphertext> &B);
        std::vector<seal::Plaintext> decryptResult(const std::vector<seal::Ciphertext> &encrypted_result);
        std::vector<std::vector<int64_t>> decodeResult(const std::vector<seal::Plaintext> &encoded_result);

    private:
        class SealBFVContext
        {
        public:
            SealBFVContext(int poly_modulus_degree);

            seal::BatchEncoder &encoder() { return *m_p_batch_encoder; }
            seal::Evaluator &evaluator() { return *m_p_evaluator; }
            seal::Decryptor &decryptor() { return *m_p_decryptor; }
            const seal::Encryptor &encryptor() const { return *m_p_encryptor; }
            const seal::PublicKey &public_key() const { return m_public_key; }
            const seal::SecretKey &secret_key() const { return m_secret_key; }
            seal::SEALContext &context() { return *m_p_seal_context; }

        private:
            std::shared_ptr<seal::SEALContext> m_p_seal_context;
            std::unique_ptr<seal::KeyGenerator> m_p_keygen;
            seal::PublicKey m_public_key;
            seal::SecretKey m_secret_key;
            std::unique_ptr<seal::Encryptor> m_p_encryptor;
            std::unique_ptr<seal::Evaluator> m_p_evaluator;
            std::unique_ptr<seal::Decryptor> m_p_decryptor;
            std::unique_ptr<seal::BatchEncoder> m_p_batch_encoder;
        };

        std::size_t m_vector_size;
        std::shared_ptr<SealBFVContext> m_p_context;

        SealBFVContext &context() { return *m_p_context; }
    };
    //! [benchmark class original_workload]

    std::shared_ptr<Workload> m_p_workload;
};

/// @endcond
