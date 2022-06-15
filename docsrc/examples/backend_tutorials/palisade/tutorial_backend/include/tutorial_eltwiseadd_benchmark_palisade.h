
// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

/// @cond

#include <array>

#include "hebench/api_bridge/cpp/hebench.hpp"
#include "palisade.h"

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

        std::vector<lbcrypto::Plaintext> encodeVector(const std::vector<std::vector<std::int64_t>> &vec);
        std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> encryptVector(const std::vector<lbcrypto::Plaintext> &encoded_vec);
        std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> eltwiseadd(const std::vector<lbcrypto::Plaintext> &A,
                                                                         const std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> &B);
        std::vector<lbcrypto::Plaintext> decryptResult(const std::vector<lbcrypto::Ciphertext<lbcrypto::DCRTPoly>> &encrypted_result);
        std::vector<std::vector<int64_t>> decodeResult(const std::vector<lbcrypto::Plaintext> &encoded_result);

    private:
        class PalisadeBFVContext
        {
        public:
            PalisadeBFVContext(int poly_modulus_degree);

            auto publicKey() const { return m_keys->publicKey; }
            std::size_t getSlotCount() const { return m_slot_count; }
            lbcrypto::CryptoContext<lbcrypto::DCRTPoly> &context() { return *m_p_palisade_context; }
            void decrypt(const lbcrypto::Ciphertext<lbcrypto::DCRTPoly> &cipher, lbcrypto::Plaintext &plain)
            {
                context()->Decrypt(m_keys->secretKey, cipher, &plain);
            }

            lbcrypto::Plaintext decrypt(const lbcrypto::Ciphertext<lbcrypto::DCRTPoly> &cipher)
            {
                lbcrypto::Plaintext retval;
                decrypt(cipher, retval);
                return retval;
            }

        private:
            std::shared_ptr<lbcrypto::CryptoContext<lbcrypto::DCRTPoly>> m_p_palisade_context;
            std::unique_ptr<lbcrypto::LPKeyPair<lbcrypto::DCRTPoly>> m_keys;
            std::size_t m_slot_count;
        };

        std::size_t m_vector_size;
        std::shared_ptr<PalisadeBFVContext> m_p_context;

        PalisadeBFVContext &context() { return *m_p_context; }
    };
    //! [benchmark class original_workload]

    std::shared_ptr<Workload> m_p_workload;
};

/// @endcond
